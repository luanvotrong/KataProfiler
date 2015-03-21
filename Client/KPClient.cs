﻿using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.IO;
using System.ComponentModel;
using System.Net;

namespace KataProfiler
{
	class KPClient
	{
		#region Singleton
		private static KPClient m_sInstance = null;
		public static KPClient getInstance()
		{
			if (m_sInstance == null)
			{
				m_sInstance = new KPClient();
			}
			return m_sInstance;
		}
		#endregion

		#region Constants
		private const string HOST = "localhost";
		private const int PORT_NUMBER = 38300;

		private const int BUFFER_SIZE			= 1024 * 1024 * 2; // 2 MB

		private const int MAX_COMMANDS_NUMBER	= 16384;

		public const int MAX_TEX_UNITS_NUMBER	= 32;

		//
		private const int REQUEST_CODE_CAPTURE			= 0;
		private const int REQUEST_CODE_MODIFY_PROGRAM	= 1;

		//
		private const int MAX_RETRY_TIME				= 10000;
		private const int READ_TIMEOUT = 10000;

		//
		private const string CONNECTION_ERROR_START_STRING = "Could not connect to the device, please try to:\n\n";
		private const string CONNECTION_ERROR_DIS_STRING = "Disconnected, please try to:\n\n";
		private const string CONNECTION_ADVICE_STRING =
					"  ● Disconnect and reconnect (USB cable) or (ADB/SDB over network).\n" +
					"  ● Kill/exit your application on the device and run it again.";
		private const string CONNECTION_GENERAL_ERROR_STRING = "Something went wrong, please try again later.";
		private const string FORWARD_TCP_ERROR_STRING = "Please check adb/sdb tool, make sure:\n\n" +
					"  ● You have adb/sdb in your PATH environment.\n" +
					"  ● Your PC has already installed USB driver for your device.\n" +
					"  ● You are connecting to only one device.\n";
		#endregion

		#region Properties
		private List<KPTexture>		m_pTexturesList		= new List<KPTexture>();
		private List<KPProgram>		m_pProgramsList		= new List<KPProgram>();
		private List<KPShader>		m_pShadersList		= new List<KPShader>();
		private List<KPVbo>			m_pVbosList			= new List<KPVbo>();

		private byte			m_activeTexUnit; // 0-31
		private uint[]			m_pTexUnitsList_2D		= new uint[MAX_TEX_UNITS_NUMBER];
		private uint[]			m_pTexUnitsList_CubeMap	= new uint[MAX_TEX_UNITS_NUMBER];
		private uint			m_usingProgram;
		private uint			m_bindingVbo_ArrayBuffer;
        private uint			m_bindingVbo_ElementArrayBuffer;
		
		private KPMessage[] m_listCommands;
		private int m_countCommands;
		public KPMessage[] ListCommands { get { return m_listCommands; } }
		public int CountCommands { get { return m_countCommands; } }

		private int[] m_listRenderCommandIndexes;
		public int[] ListRenderCommandIndexes { get { return m_listRenderCommandIndexes; } }
		private int m_countRenderCommandIndexes;
		public int CountRenderCommandIndexes { get { return m_countRenderCommandIndexes; } }

		private KPStateMachine m_currentStateMachine;
		public KPStateMachine CurrentStateMachine { get { return m_currentStateMachine; } }
		#endregion

		private static byte[] m_sBuffer = new byte[BUFFER_SIZE];
		private bool m_wantToCancelCapture;
		private Object thisLock = new Object();

		private KPClient()
		{
			gl2.init();

			m_listCommands = new KPMessage[MAX_COMMANDS_NUMBER];
			for (int i = 0; i < m_listCommands.Length; i++)
			{
				m_listCommands[i] = new KPMessage();
			}
			m_countCommands = 0;

			m_listRenderCommandIndexes = new int[MAX_COMMANDS_NUMBER];
			m_countRenderCommandIndexes = 0;

			for (int i = 0; i < m_pTexUnitsList_2D.Length; i++) m_pTexUnitsList_2D[i] = 0;
			for (int i = 0; i < m_pTexUnitsList_CubeMap.Length; i++) m_pTexUnitsList_CubeMap[i] = 0;

			m_activeTexUnit = 0;
			m_usingProgram = 0;
			m_bindingVbo_ArrayBuffer = 0;
			m_bindingVbo_ElementArrayBuffer = 0;

			//
			m_currentStateMachine = new KPStateMachine();
		}

		private void reset()
		{
			m_wantToCancelCapture = false;

			m_currentStateMachine.reset();

			m_pTexturesList.Clear();
			
			m_pProgramsList.Clear();
			
			m_pShadersList.Clear();
			
			m_pVbosList.Clear();

			for (int i = 0; i < m_pTexUnitsList_2D.Length; i++) m_pTexUnitsList_2D[i] = 0;
			for (int i = 0; i < m_pTexUnitsList_CubeMap.Length; i++) m_pTexUnitsList_CubeMap[i] = 0;

			m_activeTexUnit = 0;
			m_usingProgram = 0;
			m_bindingVbo_ArrayBuffer = 0;
			m_bindingVbo_ElementArrayBuffer = 0;

			clearCommands();

			//Utils.gc();
		}

		private void clearCommands()
		{
			for (int i = 0; i < m_countCommands; i++)
			{
				m_listCommands[i].reset();
			}
			m_countCommands = 0;

			m_countRenderCommandIndexes = 0;
		}

		private void onReceiveMessage(KPMessage msg)
		{
			KPMessageType type = (KPMessageType)msg.Type;

			if (KPMessageType.KMT_OBJECT_TEXTURE <= type && type <= KPMessageType.KMT_OBJECT_VBO)
			{
				addObject(msg);
			}

			if (KPMessageType.KMT_glActiveTexture <= type && type <= KPMessageType.KMT_glViewport)
			{
				Utils.assert(m_countCommands + 1 < m_listCommands.Length);
				m_listCommands[m_countCommands++].copyFrom(msg);
			}

			if (type == KPMessageType.KMT_STATE_CURRENT_TEXTURES_STATUS)
			{
			    MyBinStream stream = new MyBinStream(msg.Data);
				for (int i = 0; i < MAX_TEX_UNITS_NUMBER; i++)
				{
					m_pTexUnitsList_2D[i] = stream.readUInt();
					m_pTexUnitsList_CubeMap[i] = stream.readUInt();
				}
				m_activeTexUnit = stream.readByte();
				stream.close();
			}

			if (type == KPMessageType.KMT_STATE_CURRENT_USING_PROGRAM)
			{
				MyBinStream stream = new MyBinStream(msg.Data);
				m_usingProgram = stream.readUInt();
				stream.close();				
			}

			if (type == KPMessageType.KMT_STATE_CURRENT_BINDING_VBOS)
			{
				MyBinStream stream = new MyBinStream(msg.Data);
				m_bindingVbo_ArrayBuffer = stream.readUInt();
				m_bindingVbo_ElementArrayBuffer = stream.readUInt();
				stream.close();
			}
		}

		private void processResult()
		{
			#region Render commands

			// Append a dummy command
			Utils.assert(m_countCommands + 1 < m_listCommands.Length);
			m_listCommands[m_countCommands].reset();
			m_listCommands[m_countCommands].Type = (int)KPMessageType.KMT_NONE;
			m_countCommands++;

			//
			m_countRenderCommandIndexes = 0;

			for (int i = 0; i < m_countCommands; i++)
			{
				KPMessage msg = m_listCommands[i];
				KPMessageType type = (KPMessageType)msg.Type;

				if (type == KPMessageType.KMT_glClear || type == KPMessageType.KMT_glDrawArrays ||
					type == KPMessageType.KMT_glDrawElements || type == KPMessageType.KMT_NONE)
				{
					Utils.assert(m_countRenderCommandIndexes + 1 < m_listRenderCommandIndexes.Length);
					m_listRenderCommandIndexes[m_countRenderCommandIndexes++] = i;
				}
			}
			#endregion

			#region Init lifetime
			//for (int i = 0; i < m_pTexturesList.Length; i++) if (m_pTexturesList[i].Id > 0)
			//{
			//    m_pTexturesList[i].LifeTimeBegin = 0;
			//    m_pTexturesList[i].LifeTimeEnd = m_countCommands - 1;
			//}
			foreach (KPTexture tex in m_pTexturesList)
			{
				tex.LifeTimeBegin = 0;
				tex.LifeTimeEnd = m_countCommands - 1;
			}

			//for (int i = 0; i < m_pProgramsList.Length; i++) if (m_pProgramsList[i].Id > 0)
			//{
			//    m_pProgramsList[i].LifeTimeBegin = 0;
			//    m_pProgramsList[i].LifeTimeEnd = m_countCommands - 1;
			//}
			foreach (KPProgram prog in m_pProgramsList)
			{
				prog.LifeTimeBegin = 0;
				prog.LifeTimeEnd = m_countCommands - 1;
			}
			//for (int i = 0; i < m_pShadersList.Length; i++) if (m_pShadersList[i].Id > 0)
			//{
			//    m_pShadersList[i].LifeTimeBegin = 0;
			//    m_pShadersList[i].LifeTimeEnd = m_countCommands - 1;
			//}
			foreach (KPShader shader in m_pShadersList)
			{
				shader.LifeTimeBegin = 0;
				shader.LifeTimeEnd = m_countCommands - 1;
			}
			//for (int i = 0; i < m_pVbosList.Length; i++) if (m_pVbosList[i].Id > 0)
			//{
			//    m_pVbosList[i].LifeTimeBegin = 0;
			//    m_pVbosList[i].LifeTimeEnd = m_countCommands - 1;
			//}
			foreach (KPVbo vbo in m_pVbosList)
			{
				vbo.LifeTimeBegin = 0;
				vbo.LifeTimeEnd = m_countCommands - 1;
			}
			#endregion

			for (int i = 0; i < m_countCommands; i++)
			{
				KPMessage msg = m_listCommands[i];
				KPMessageType type = (KPMessageType)msg.Type;

				switch (type)
				{
					#region Texture
					case KPMessageType.KMT_glBindTexture:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();

						if (error == 0)
						{
							uint target = stream.readUInt();
							uint texture = stream.readUInt();
							uint actual = stream.readUInt();

							if (actual > 0)
							{
								KPTexture texOld = getTexture(actual, i);
								if (texOld.TexType == KPTextureType.TEX_NONE)
								{
									KPTexture tex = getFreeTexture();
									tex.copyFrom(texOld);
									texOld.LifeTimeEnd = i - 1;

									tex.Id = actual;
									tex.LifeTimeBegin = i;
									tex.LifeTimeEnd = m_countCommands - 1;
									tex.TexType = target == gl2.GL_TEXTURE_2D ? KPTextureType.TEX_2D : KPTextureType.TEX_CUBE_MAP;
								}
							}
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glGenTextures:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						int n = stream.readInt();
						for (int j = 0; j < n; j++)
						{
							KPTexture tex = getFreeTexture();

							tex.Id = stream.readUInt();
							tex.LifeTimeBegin = i;
							tex.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glTexImage2D:
					{
						MyBinStream stream = new MyBinStream(msg.Data);

						byte error = stream.readByte();

						if (error == 0)
						{
							uint id = stream.readUInt();
							uint target = stream.readUInt();

							int level = stream.readInt();
							int internalformat = stream.readInt();
							int width = stream.readInt();
							int height = stream.readInt();
							int border = stream.readInt();
							int format = stream.readInt();
							uint datatype = stream.readUInt();
							int address = stream.readInt();

							KPTexture tex = getFreeTexture();
							KPTexture texOld = getTexture(id, i);
							tex.copyFrom(texOld);
							texOld.LifeTimeEnd = i - 1;

							tex.Id = id;
							tex.LifeTimeBegin = i;
							tex.LifeTimeEnd = m_countCommands - 1;
							tex.on_glTexImage2D(level, internalformat, width, height,
								border, (uint)internalformat, datatype, msg.Data, stream.CurrentPosition);
						}
						
						stream.close();
						break;
					}
					case KPMessageType.KMT_glCompressedTexImage2D:
					{
						MyBinStream stream = new MyBinStream(msg.Data);

						byte error = stream.readByte();

						if (error == 0)
						{
							uint id = stream.readUInt();
							uint target = stream.readUInt();

							int level = stream.readInt();
							uint internalformat = stream.readUInt();
							int width = stream.readInt();
							int height = stream.readInt();
							int border = stream.readInt();
							int imageSize = stream.readInt();
							int dataAddress = stream.readInt();

							KPTexture tex = getFreeTexture();
							KPTexture texOld = getTexture(id, i);
							tex.copyFrom(texOld);
							texOld.LifeTimeEnd = i - 1;

							tex.Id = id;
							tex.LifeTimeBegin = i;
							tex.LifeTimeEnd = m_countCommands - 1;
							tex.on_glCompressedTexImage2D(level, internalformat, width, height, border, imageSize,
								msg.Data, stream.CurrentPosition);
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glTexSubImage2D:
					{
						MyBinStream stream = new MyBinStream(msg.Data);

						byte error = stream.readByte();

						if (error == 0)
						{
							uint id = stream.readUInt();
							uint target = stream.readUInt();

							int level = stream.readInt();
							int xoffset = stream.readInt();
							int yoffset = stream.readInt();
							int width = stream.readInt();
							int height = stream.readInt();
							uint format = stream.readUInt();
							uint datatype = stream.readUInt();
							int dataAddress = stream.readInt();

							KPTexture tex = getFreeTexture();
							KPTexture texOld = getTexture(id, i);
							tex.copyFrom(texOld);
							texOld.LifeTimeEnd = i - 1;

							tex.Id = id;
							tex.LifeTimeBegin = i;
							tex.LifeTimeEnd = m_countCommands - 1;
							tex.on_glTexSubImage2D(level, xoffset, yoffset, width, height,
													format, datatype, msg.Data, stream.CurrentPosition);
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glCompressedTexSubImage2D:
					{
						// TODO
						break;
					}
					case KPMessageType.KMT_glDeleteTextures:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						int n = stream.readInt();
						for (int j = 0; j < n; j++)
						{
							uint id = stream.readUInt();
							KPTexture tex = getTexture(id, i);
							tex.LifeTimeEnd = i - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glCopyTexImage2D:
					{
						MyBinStream stream = new MyBinStream(msg.Data);

						byte error = stream.readByte();

						if (error == 0)
						{
							uint id = stream.readUInt();
							uint target = stream.readUInt();

							int level = stream.readInt();
							uint internalformat = stream.readUInt();
							int x = stream.readInt();
							int y = stream.readInt();
							int width = stream.readInt();
							int height = stream.readInt();
							int border = stream.readInt();

							KPTexture tex = getFreeTexture();
							KPTexture texOld = getTexture(id, i);
							tex.copyFrom(texOld);
							texOld.LifeTimeEnd = i - 1;

							tex.Id = id;
							tex.LifeTimeBegin = i;
							tex.LifeTimeEnd = m_countCommands - 1;
							tex.on_glTexImage2D(level, (int)internalformat, width, height,
								border, internalformat, gl2.GL_UNSIGNED_BYTE, msg.Data, stream.CurrentPosition);
						}
						
						stream.close();
						break;
					}
					case KPMessageType.KMT_glCopyTexSubImage2D:
					{
						MyBinStream stream = new MyBinStream(msg.Data);

						byte error = stream.readByte();

						if (error == 0)
						{
							uint id = stream.readUInt();
							uint target = stream.readUInt();

							int level = stream.readInt();
							int xoffset = stream.readInt();
							int yoffset = stream.readInt();
							int x = stream.readInt();
							int y = stream.readInt();
							int width = stream.readInt();
							int height = stream.readInt();

							KPTexture tex = getFreeTexture();
							KPTexture texOld = getTexture(id, i);
							tex.copyFrom(texOld);
							texOld.LifeTimeEnd = i - 1;

							tex.Id = id;
							tex.LifeTimeBegin = i;
							tex.LifeTimeEnd = m_countCommands - 1;
							tex.on_glTexSubImage2D(level, xoffset, yoffset, width, height,
									(uint)tex.Mipmaps[level].Format, gl2.GL_UNSIGNED_BYTE, msg.Data, stream.CurrentPosition);
						}

						stream.close();
						break;
					}
					#endregion

					#region Program
					#region Program 1
					case KPMessageType.KMT_glCreateProgram:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint id = stream.readUInt();
							KPProgram prog = getFreeProgram();
							prog.Id = id;
							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glAttachShader:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();
							uint shaderId = stream.readUInt();
						
							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;
						
							prog.Id = progId;
							KPShader shader = getShader(shaderId, i);
							Utils.assert(shader != null);

							if (shader.Type == gl2.GL_VERTEX_SHADER) prog.VsId = shaderId;
							else prog.FsId = shaderId;

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glLinkProgram:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						stream.close();

						if (error == 0)
						{
							KPProgram prog = getFreeProgram();
							prog.fromBytes(msg.Data, 1 + 4, msg.Length - 1 - 4);

							uint id = prog.Id;
							prog.Id = 0;
						
							KPProgram progOld = getProgram(id, i);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = id;
							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}
						
						break;
					}
					case KPMessageType.KMT_glDetachShader:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();

						if (error == 0)
						{
							uint progId = stream.readUInt();
							uint shaderId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							KPShader shader = getShader(shaderId, i);
							Utils.assert(shader != null);

							if (shader.Type == gl2.GL_VERTEX_SHADER) prog.VsId = 0;
							else prog.FsId = 0;

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glDeleteProgram:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getProgram(progId, i);
							prog.LifeTimeEnd = i - 1;
						}
						stream.close();
						break;
					}
					#endregion

					//=========================================================================================
					#region glUniform

					#region glUniform 1
					case KPMessageType.KMT_glUniform1f:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform1f(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform1fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform1fv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform1i:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform1i(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform1iv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform1iv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					#endregion

					#region glUniform 2
					case KPMessageType.KMT_glUniform2f:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform2f(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform2fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform2fv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;

						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform2i:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform2i(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform2iv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform2iv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					#endregion

					#region glUniform 3
					case KPMessageType.KMT_glUniform3f:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform3f(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform3fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform3fv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform3i:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform3i(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform3iv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform3iv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					#endregion

					#region glUniform 4
					case KPMessageType.KMT_glUniform4f:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform4f(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform4fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform4fv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform4i:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform4i(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniform4iv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniform4iv(stream);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					#endregion

					#region glUniform matrix
					case KPMessageType.KMT_glUniformMatrix2fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniformMatrix(stream, 2);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniformMatrix3fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniformMatrix(stream, 3);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glUniformMatrix4fv:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint progId = stream.readUInt();

							KPProgram prog = getFreeProgram();
							KPProgram progOld = getProgram(progId, i);

							prog.copyFrom(progOld);
							progOld.LifeTimeEnd = i - 1;

							prog.Id = progId;
							prog.on_glUniformMatrix(stream, 4);

							prog.LifeTimeBegin = i;
							prog.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					#endregion

					#endregion
					//=========================================================================================

					#endregion

					#region Shader
					case KPMessageType.KMT_glCreateShader:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint shaderType = stream.readUInt();
							uint id = stream.readUInt();
							
							KPShader shader = getFreeShader();
							shader.Id = id;
							shader.Type = shaderType;

							shader.LifeTimeBegin = i;
							shader.LifeTimeEnd = m_countCommands - 1;
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glDeleteShader:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint id = stream.readUInt();
							KPShader shader = getShader(id, i);
							shader.LifeTimeEnd = i - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glShaderSource:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						byte error = stream.readByte();
						if (error == 0)
						{
							uint shaderId = stream.readUInt();
							string source = stream.readString();

							KPShader shader = getFreeShader();
							KPShader shaderOld = getShader(shaderId, i);

							shader.copyFrom(shaderOld);
							shaderOld.LifeTimeEnd = i - 1;

							shader.Id = shaderId;
							shader.Source = source;

							shader.LifeTimeBegin = i;
							shader.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					#endregion

					#region Vbo
					case KPMessageType.KMT_glGenBuffers:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						int n = stream.readInt();
						for (int j = 0; j < n; j++)
						{
							KPVbo vbo = getFreeVbo();
							vbo.Id = stream.readUInt();
							Utils.assert(vbo.Id > 0);
							vbo.LifeTimeBegin = i;
							vbo.LifeTimeEnd = m_countCommands - 1;
						}
						stream.close();
						break;
					}
					case KPMessageType.KMT_glBufferData:
					{
						MyBinStream stream = new MyBinStream(msg.Data);

						byte error = stream.readByte();
						if (error == 0)
						{
							uint id = stream.readUInt();
							uint target = stream.readUInt();
							int size = stream.readInt();
							int dataAddress = stream.readInt();
							uint usage = stream.readUInt();

							KPVbo vbo = getFreeVbo();
							KPVbo vboOld = getVbo(id, i);
							vbo.copyFrom(vboOld);
							vboOld.LifeTimeEnd = i - 1;

							vbo.Id = id;
							vbo.LifeTimeBegin = i;
							vbo.LifeTimeEnd = m_countCommands - 1;

							vbo.on_glBufferData(size, dataAddress, usage);
						}

						stream.close();
						break;
					}
					case KPMessageType.KMT_glBufferSubData:
					{
						// Don't care
						break;
					}
					case KPMessageType.KMT_glDeleteBuffers:
					{
						MyBinStream stream = new MyBinStream(msg.Data);
						int n = stream.readInt();
						for (int j = 0; j < n; j++)
						{
							uint id = stream.readUInt();
							KPVbo vbo = getVbo(id, i);
							vbo.LifeTimeEnd = i - 1;
						}
						stream.close();
						break;
					}
					#endregion
				}
			}
		}

		public bool captureFrame(BackgroundWorker worker = null) // return true means cancel
		{
			reset();

			if (!Utils.runForwardCmd(PORT_NUMBER))
			{
				throw new Exception(FORWARD_TCP_ERROR_STRING);
			}

			TcpClient tcpClient = new TcpClient();
			Stream stream = null;
			
			try
			{
				tcpClient.Connect(HOST, PORT_NUMBER);
				stream = tcpClient.GetStream();
			}
			catch (Exception)
			{
				throw new Exception(CONNECTION_ERROR_START_STRING + CONNECTION_ADVICE_STRING);
			}
			
			stream.ReadTimeout = READ_TIMEOUT;

			byte[] bytes = new byte[4];
			Utils.int2bytes(REQUEST_CODE_CAPTURE, bytes);
			stream.Write(bytes, 0, 4);

			KPMessage msg = new KPMessage();
			bool finish = false;

			int oldTicks = -1;
			bool isCancel = false;

			string error = null;

			while (!finish)
			{
				int len = 0;
				try
				{
					len = stream.Read(m_sBuffer, 0, m_sBuffer.Length);
				}
				catch (Exception)
				{
					error = CONNECTION_ERROR_DIS_STRING + CONNECTION_ADVICE_STRING;
					break;
				}

				lock (thisLock)
				{
					if (m_wantToCancelCapture)
					{
						isCancel = true;
						break;
					}
				}

				if (len <= 0)
				{
					if (oldTicks == -1)
					{
						oldTicks = System.Environment.TickCount;
					}
					else
					{
						int delta = System.Environment.TickCount - oldTicks;
						if (delta > MAX_RETRY_TIME)
						{
							error = CONNECTION_ERROR_DIS_STRING + CONNECTION_ADVICE_STRING;
							break;
						}
					}
				}
				else
				{
					oldTicks = -1;
				}

				int offset = 0;

				while (true)
				{
					int k = msg.consume(m_sBuffer, offset, len - 1);
					if (msg.isFull())
					{
						if (msg.isRequestFinishMessage())
						{
							finish = true;
							break;
						}

						if (msg.isRequestErrorMessage())
						{
							if (msg.Length > 0)
							{
								MyBinStream str = new MyBinStream(msg.Data);
								error = str.readString();
								str.close();
							}
							else
							{
								error = CONNECTION_GENERAL_ERROR_STRING;
							}
							finish = true;
							break;
						}

						onReceiveMessage(msg);

						if (worker != null) worker.ReportProgress(0, msg);

						bool isGc =		msg.Type == (int)KPMessageType.KMT_glDrawElements ||
										msg.Type == (int)KPMessageType.KMT_glDrawArrays ||
										msg.Type == (int)KPMessageType.KMT_glClear;

						msg.reset();

						if (isGc)
						{
							Utils.gc();
						}
					}
					offset += k;
					Utils.assert(offset <= len);
					if (offset == len) break;
				}
			}

			stream.Close();
			tcpClient.Close();

			stream = null;
			tcpClient = null;

			if (isCancel)
			{
				reset();
				return true;
			}

			if (error != null)
			{
				reset();
				throw new Exception(error);
			}

			//Utils.gc();

			processResult();

			//Utils.gc();

			return false;
		}

		public void cancelCaptureFrame()
		{
			lock (thisLock)
			{
				m_wantToCancelCapture = true;
			}
		}

		public void modifyProgram(uint progId, uint vsId, uint fsId, string vsSource, string fsSource)
		{
			if (!Utils.runForwardCmd(PORT_NUMBER))
			{
				throw new Exception(FORWARD_TCP_ERROR_STRING);
			}

			TcpClient tcpClient = new TcpClient();
			Stream stream = null;

			try
			{
				tcpClient.Connect(HOST, PORT_NUMBER);
				stream = tcpClient.GetStream();
			}
			catch (Exception)
			{
				throw new Exception(CONNECTION_ERROR_START_STRING + CONNECTION_ADVICE_STRING);
			}

			stream.ReadTimeout = READ_TIMEOUT;

			byte[] bytes = new byte[4];
			
			Utils.int2bytes(REQUEST_CODE_MODIFY_PROGRAM, bytes);	stream.Write(bytes, 0, 4);

			Utils.uint2bytes(progId, bytes);						stream.Write(bytes, 0, 4);
			Utils.uint2bytes(vsId, bytes);							stream.Write(bytes, 0, 4);
			Utils.uint2bytes(fsId, bytes);							stream.Write(bytes, 0, 4);

			int sourceLen = vsSource.Length;
			Utils.int2bytes(sourceLen, bytes); stream.Write(bytes, 0, 4);
			if (sourceLen > 0)
			{
				bytes = new byte[sourceLen];
				Utils.getBytesFromString(vsSource, bytes);
				stream.Write(bytes, 0, sourceLen);
			}

			sourceLen = fsSource.Length;
			Utils.int2bytes(sourceLen, bytes); stream.Write(bytes, 0, 4);
			if (sourceLen > 0)
			{
				bytes = new byte[sourceLen];
				Utils.getBytesFromString(fsSource, bytes);
				stream.Write(bytes, 0, sourceLen);
			}

			bytes = null;

			//------------------------------------------------------------------------------------------

			KPMessage msg = new KPMessage();
			bool finish = false;

			string error = null;
			int oldTicks = -1;

			while (!finish)
			{
				int len = 0;
				try
				{
					len = stream.Read(m_sBuffer, 0, m_sBuffer.Length);
				}
				catch (Exception ex)
				{
					error = ex.Message;
					break;
				}

				if (len <= 0)
				{
					if (oldTicks == -1)
					{
						oldTicks = System.Environment.TickCount;
					}
					else
					{
						int delta = System.Environment.TickCount - oldTicks;
						if (delta > MAX_RETRY_TIME)
						{
							error = CONNECTION_ERROR_DIS_STRING + CONNECTION_ADVICE_STRING;
							break;
						}
					}
				}
				else
				{
					oldTicks = -1;
				}

				int offset = 0;

				while (true)
				{
					int k = msg.consume(m_sBuffer, offset, len - 1);
					if (msg.isFull())
					{
						if (msg.isRequestFinishMessage())
						{
							finish = true;
							break;
						}

						if (msg.isRequestErrorMessage())
						{
							if (msg.Length > 0)
							{
								MyBinStream str = new MyBinStream(msg.Data);
								error = str.readString();
								str.close();
							}
							else
							{
								error = CONNECTION_GENERAL_ERROR_STRING;
							}
							finish = true;
							break;
						}

						Utils.assert(false);

						msg.reset();
					}
					offset += k;
					Utils.assert(offset <= len);
					if (offset == len) break;
				}
			}

			stream.Close();
			tcpClient.Close();

			if (error != null)
			{
				throw new Exception(error);
			}
		}

		public void makeStateMachine(int indexCommand)
		{
			m_currentStateMachine.reset();

			#region Objects list
			List<KPTexture> listTextures = m_currentStateMachine.ListTextures;
			foreach (KPTexture tex in m_pTexturesList)
			{
				if (tex.isLiveAt(indexCommand))
				{
					listTextures.Add(tex);
				}
			}

			List<KPProgram> listPrograms = m_currentStateMachine.ListPrograms;
			foreach (KPProgram prog in m_pProgramsList)
			{
				if (prog.isLiveAt(indexCommand))
				{
					listPrograms.Add(prog);
				}
			}

			List<KPShader> listShaders = m_currentStateMachine.ListShaders;
			foreach (KPShader shader in m_pShadersList)
			{
				if (shader.isLiveAt(indexCommand))
				{
					listShaders.Add(shader);
				}
			}

			List<KPVbo> listVbos = m_currentStateMachine.ListVbos;
			foreach (KPVbo vbo in m_pVbosList)
			{
				if (vbo.isLiveAt(indexCommand))
				{
					listVbos.Add(vbo);
				}
			}
			#endregion

			#region Other current states
			m_currentStateMachine.CurrentActiveTexUnit				= m_activeTexUnit;
			m_currentStateMachine.CurrentProgId						= m_usingProgram;
			m_currentStateMachine.CurrentVboId_ArrayBuffer			= m_bindingVbo_ArrayBuffer;
			m_currentStateMachine.CurrentVboId_ElementArrayBuffer	= m_bindingVbo_ElementArrayBuffer;

			uint[] smListTu_2D = m_currentStateMachine.ListTexUnits_2D;
			for (int i = 0; i < smListTu_2D.Length; i++) smListTu_2D[i] = m_pTexUnitsList_2D[i];

			uint[] smListTu_CubeMap = m_currentStateMachine.ListTexUnits_CubeMap;
			for (int i = 0; i < smListTu_CubeMap.Length; i++) smListTu_CubeMap[i] = m_pTexUnitsList_CubeMap[i];

			for (int i = 0; i <= indexCommand; i++)
			{
				KPMessage msg = m_listCommands[i];
				KPMessageType type = (KPMessageType)msg.Type;

				if (type == KPMessageType.KMT_glActiveTexture)
				{
					MyBinStream stream = new MyBinStream(msg.Data);
					stream.skip(4);
					m_currentStateMachine.CurrentActiveTexUnit = stream.readByte();
					stream.close();
				}
				else if (type == KPMessageType.KMT_glBindTexture)
				{
					MyBinStream stream = new MyBinStream(msg.Data);
					byte error = stream.readByte();
					uint target = stream.readUInt();
					uint textureId = stream.readUInt();
					uint actualTextureId = stream.readUInt();
					stream.close();

					if (target == gl2.GL_TEXTURE_2D)
					{
						m_currentStateMachine.CurrentTexId_2D = actualTextureId;
					}
					else if (target == gl2.GL_TEXTURE_CUBE_MAP)
					{
						m_currentStateMachine.CurrentTexId_CubeMap = actualTextureId;
					}
				}
				else if (type == KPMessageType.KMT_glUseProgram)
				{
					MyBinStream stream = new MyBinStream(msg.Data);
					stream.skip(4);
					m_currentStateMachine.CurrentProgId = stream.readUInt();
					stream.close();
				}
				else if (type == KPMessageType.KMT_glBindBuffer)
				{
					MyBinStream stream = new MyBinStream(msg.Data);
					byte error = stream.readByte();
					uint target = stream.readUInt();
					uint bufferId = stream.readUInt();
					uint actualBoundBufferId = stream.readUInt();
					stream.close();

					if (target == gl2.GL_ARRAY_BUFFER)
					{
						m_currentStateMachine.CurrentVboId_ArrayBuffer = actualBoundBufferId;
					}
					else if (target == gl2.GL_ELEMENT_ARRAY_BUFFER)
					{
						m_currentStateMachine.CurrentVboId_ElementArrayBuffer = actualBoundBufferId;
					}
				}
			}
			#endregion
		}

		private KPTexture getFreeTexture()
		{
			KPTexture tex = new KPTexture();
			m_pTexturesList.Add(tex);

			return tex;
		}

		private KPTexture getTexture(uint id, int indexCommand)
		{
			foreach (KPTexture tex in m_pTexturesList)
			{
				if (tex.Id == id && tex.isLiveAt(indexCommand))
				{
					return tex;
				}
			}
			Utils.assert(false);
			return null;
		}

		private KPProgram getFreeProgram()
		{
			KPProgram prog = new KPProgram();
			m_pProgramsList.Add(prog);

			return prog;
		}

		private KPProgram getProgram(uint id, int indexCommand)
		{
			foreach (KPProgram prog in m_pProgramsList)
			{
				if (prog.Id == id && prog.isLiveAt(indexCommand))
				{
					return prog;
				}
			}

			Utils.assert(false);
			return null;
		}

		private KPShader getFreeShader()
		{
			KPShader shader = new KPShader();
			m_pShadersList.Add(shader);

			return shader;
		}

		private KPShader getShader(uint id, int indexCommand)
		{
			foreach (KPShader shader in m_pShadersList)
			{
				if (shader.Id == id && shader.isLiveAt(indexCommand))
				{
					return shader;
				}
			}

			Utils.assert(false);
			return null;
		}

		private KPVbo getFreeVbo()
		{
			KPVbo vbo = new KPVbo();
			m_pVbosList.Add(vbo);

			return vbo;
		}

		private KPVbo getVbo(uint id, int indexCommand)
		{
			foreach (KPVbo vbo in m_pVbosList)
			{
				if (vbo.Id == id && vbo.isLiveAt(indexCommand))
				{
					return vbo;
				}
			}

			Utils.assert(false);
			return null;
		}

		private void addObject(KPMessage msg)
		{
			KPMessageType type = (KPMessageType)msg.Type;
			switch (type)
			{
				case KPMessageType.KMT_OBJECT_PROGRAM:
				{
					KPProgram prog = getFreeProgram();
					prog.fromMessage(msg);
					break;
				}

				case KPMessageType.KMT_OBJECT_SHADER:
				{
					KPShader shader = getFreeShader();
					shader.fromMessage(msg);
					break;
				}

				case KPMessageType.KMT_OBJECT_TEXTURE:
				{
					KPTexture tex = getFreeTexture();
					tex.fromMessage(msg);
					break;
				}

				case KPMessageType.KMT_OBJECT_VBO:
				{
					KPVbo vbo = getFreeVbo();
					vbo.fromMessage(msg);
					break;
				}
			}
			
		}
		
	}
}
