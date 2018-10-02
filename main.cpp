/*
 * main.cpp
 *
 *  Created on: Sep 24, 2018
 *      Author: HungNV
 */

#define GL_GLEXT_PROTOTYPES
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>

#ifdef __cplusplus
}
#endif

// for OpenGL
GLuint vertexShader;
GLuint fragmentShader;
GLuint program;

bool supports_npot;
bool multiTexture;
bool stretchScreen;
bool support_vsync;
GLuint textures[3];

GLint linked;

int CheckGLError(const char *file, int line) {
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        const GLubyte* sError = gluErrorString(glErr);

        if (sError)
            std::cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << std::endl;
        else
        	std::cout << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << std::endl;

        retCode = 1;
        glErr = glGetError();
    }

    return retCode;
}

#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)

void loadFile(const char *fn, std::string &str)
{
    std::ifstream in(fn);
    if(!in.is_open()){
        return;
    }
    char tmp[300];
    while(!in.eof()){
        in.getline(tmp,300);
        str += tmp;
        str += "\n";
    }// End function
}

void loadShader() {
	std::cout << "loading shaders ..." << std::endl;
    glClearColor(0.5f, 0.5f, 1.0f, 0.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    //create vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //create fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexSourceCode;
    std::string fragmentSourceCode;

    loadFile("./vertex.shader", vertexSourceCode);
    loadFile("./fragment.shader", fragmentSourceCode);

    const char* adapter = vertexSourceCode.c_str();

    //attach source code to shader
    glShaderSourceARB(vertexShader, 1, &adapter, NULL);
    adapter = fragmentSourceCode.c_str();
    glShaderSourceARB(fragmentShader, 1, &adapter, NULL);

    //compile source code
    glCompileShaderARB(vertexShader);
    glCompileShaderARB(fragmentShader);

    //ok now, link into program
    program = glCreateProgram();

    glAttachShader(program, vertexShader);
    CHECK_GL_ERROR();
    glAttachShader(program, fragmentShader);
    CHECK_GL_ERROR();

    glLinkProgram(program);
    CHECK_GL_ERROR();

    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
    	std::cout << "cannot link program" << std::endl;
        glDeleteObjectARB(vertexShader);
        glDeleteShader(vertexShader);
        glDeleteObjectARB(fragmentShader);
        glDeleteShader(fragmentShader);
        return;
    }

    //create texture
    glGenTextures(3, textures);
}

#ifndef HasExtension
bool HasExtension(const char* src, const char* sub) {
	return (strstr(src,sub) != 0);
}
#endif

int InitGL() {// All Setup For OpenGL Goes Here
	glShadeModel(GL_SMOOTH);				// Enables Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Black Background
	glClearDepth(1.0f);						// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);					// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);

	if (extensions) {
		supports_npot = HasExtension(extensions, "GL_ARB_texture_non_power_of_two") ||
				HasExtension(extensions, "GL_APPLE_texture_2D_limited_npot");

		multiTexture = HasExtension(extensions, "GL_ARB_multitexture");

		support_vsync = HasExtension(extensions, "GLX_MESA_swap_control");
	} else {
		supports_npot = false;
		multiTexture = false;
		support_vsync = false;
	}

	loadShader();
	return GL_TRUE;							// Initialization Went OK
}

unsigned long w, h, ws_w, ws_h;
unsigned char* localVideoBufferDrawer;
unsigned char* in_buff;
#define INBUF_SIZE 1024*1024

int DrawGLScene() {	// Here's Where We Do All The Drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear The Screen And The Depth Buffer
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluPerspective(45.0, aspect, 1.0, 100.0);
    //bind texture with data
    glEnable(GL_TEXTURE_2D);

    for (int idx = 0; idx < 3; idx++) {

    	if (multiTexture)
    		glActiveTextureARB(GL_TEXTURE0_ARB+idx);

    	uint8_t* tmp = localVideoBufferDrawer;
    	if (idx == 1)
    		tmp += w*h;
    	else if (idx == 2)
    		tmp += w*h*5/4;

    	glBindTexture(GL_TEXTURE_2D, textures[idx]);
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (idx > 0 ? (w/2) : w), (idx > 0 ? (h/2) : h),
    			0, GL_RED, GL_UNSIGNED_BYTE, tmp);

    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtering
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtering
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);

    glBegin(GL_QUADS);

    if (stretchScreen) {
    	glTexCoord2f(0.0, 0.0); glVertex2d(-1.0, 1.0);
    	glTexCoord2f(0.0, 1.0); glVertex2d(-1.0, -1.0);
    	glTexCoord2f(1.0, 1.0); glVertex2d(1.0, -1.0);
    	glTexCoord2f(1.0, 0.0); glVertex2d(1.0, 1.0);
    } else {

    	float i_ratio = (float)w / (float)h;
    	float s_ratio = (float)ws_w / (float)ws_h;

    	float offset;

    	if (i_ratio > s_ratio) {
    		offset = (1.0 - (float)ws_w/(i_ratio*ws_h)) / 2.0;
    		glTexCoord2f(0.0, 0.0); glVertex2d(-1.0, 1.0 - offset);
    		glTexCoord2f(0.0, 1.0); glVertex2d(-1.0, -1.0 + offset);
    		glTexCoord2f(1.0, 1.0); glVertex2d(1.0, -1.0 + offset);
    		glTexCoord2f(1.0, 0.0); glVertex2d(1.0, 1.0 - offset);
    	} else if (i_ratio < s_ratio) {
    		offset = (1.0 - (i_ratio*ws_h)/((float)ws_w)) / 2.0;
    		glTexCoord2f(0.0, 0.0); glVertex2d(-1.0 + offset, 1.0);
    		glTexCoord2f(0.0, 1.0); glVertex2d(-1.0 + offset, -1.0);
    		glTexCoord2f(1.0, 1.0); glVertex2d(1.0 - offset, -1.0);
    		glTexCoord2f(1.0, 0.0); glVertex2d(1.0 - offset, 1.0);
    	} else {
    		glTexCoord2f(0.0, 0.0); glVertex2d(-1.0, 1.0);
    		glTexCoord2f(0.0, 1.0); glVertex2d(-1.0, -1.0);
    		glTexCoord2f(1.0, 1.0); glVertex2d(1.0, -1.0);
    		glTexCoord2f(1.0, 0.0); glVertex2d(1.0, 1.0);
    	}
    }

    glEnd();
    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
    glFlush();
    return GL_TRUE;                                // Everything Went OK
}

int RenderAVFrame(AVFrame* frame, int &justifiedHeight) {
	if((w != frame->width) || (h != frame->height)) {
		w = frame->width;
		w = frame->height;

		if(localVideoBufferDrawer)
			delete[] localVideoBufferDrawer;

		localVideoBufferDrawer = new unsigned char[w*h*3/2];
		if(!localVideoBufferDrawer)
			return -2; // got error
	}

	unsigned char *dst = localVideoBufferDrawer;
	justifiedHeight = ((h == 1088) ? 1080 : h);

	if (frame->format == AV_PIX_FMT_YUV420P ||
			frame->format == AV_PIX_FMT_YUVJ420P) {
		for (int i=0; i<3; i ++) {
			const unsigned char *src = frame->data[i];

			int dst_stride = i ? w >> 1 : w;
			int src_stride = frame->linesize[i];
			int h = i ? justifiedHeight >> 1 : justifiedHeight;

			if (src_stride == dst_stride) {
				memcpy(dst, src, dst_stride*h);
				dst += dst_stride*h;
			} else {
				while (h--) {
					memcpy(dst, src, dst_stride);
					dst += dst_stride;
					src += src_stride;
				}
			}
		}
	} else if (frame->format == AV_PIX_FMT_NV12) {
		const unsigned char *src = frame->data[0];

		int dst_stride = w;
		int src_stride = frame->linesize[0];
		int h = justifiedHeight;

		if (src_stride == dst_stride) {
			memcpy(dst, src, dst_stride*h);
			dst += dst_stride*h;
		} else {
			while (h--) {
				memcpy(dst, src, dst_stride);
				dst += dst_stride;
				src += src_stride;
			}
		}

		unsigned char *u,*v;
		h = justifiedHeight >> 1;
		dst_stride >>=1;
		src_stride = frame->linesize[1];
		u = dst;
		v = dst + h*dst_stride;
		src = frame->data[1];
		while (h--) {
			for(int i = 0; i < dst_stride; i++) {
				u[i] = src[i*2];
				v[i] = src[i*2+1];
			}
			u += dst_stride;
			v += dst_stride;
			src += src_stride;
		}
	}
	return 0;
}

void reshape(int w, int h)
{
    ws_w = w;
    ws_h = h;
    glViewport(0, 0, w, h);
}

int main(int argc, char** argv) {
	std::cout << "Program is starting now ..." << std::endl;
	av_register_all();
	glutInit(&argc, argv);
	ws_w = 640;
	ws_h = 360;
	glutInitWindowSize(ws_w, ws_h);
	int window = glutCreateWindow("Sample");
	glutReshapeFunc(reshape);
	InitGL();

	AVFormatContext* input = NULL;
	const AVCodec *codec;
	AVCodecParserContext *parser;
	AVCodecContext *c = NULL;
	AVFrame *frame;
	AVPacket *pkt;
	int strm_idx = -1;

	AVDictionary *options = NULL;
	const char* url;
	if (argc == 1) {
//		url = "rtsp://admin:123456@172.20.73.42";
		url = "rtsp://admin:123456@172.20.73.42/media/video2";
	} else {
		url = argv[1];
	}
	std::cout << "Playing url = \"" << url << "\" ..." << std::endl;
	av_dict_set(&options, "stimeout", "5000000", 0); // 5 seconds in microseconds
	int ret = avformat_open_input(&input, url, NULL, &options);
	av_dict_free(&options);

	if (ret < 0) {
		//		LOG_ERROR("Cam " << m_Camera->m_Id.c_str() << " avformat_open_input fail");
		return false;
	}

	ret = avformat_find_stream_info(input, NULL);
	if(ret < 0) {
		avformat_close_input(&input);
		//		LOG_ERROR("Cam " << m_Camera->m_Id.c_str() << " avformat_find_stream_info fail");
		return -1;
	}


	for(unsigned i = 0; i < input->nb_streams; i++) {
		AVStream* in_strm = input->streams[i];
		if (in_strm->codecpar->codec_type != AVMEDIA_TYPE_VIDEO || in_strm->codecpar->codec_id != AV_CODEC_ID_H264) {
			continue;
		}
		strm_idx = i;
		codec = avcodec_find_decoder_by_name("h264_mmal");
		if (!codec)
			codec = avcodec_find_decoder_by_name("h264");
		if (!codec) {
			strm_idx = -1;
			continue;
		}

		pkt = av_packet_alloc();
		parser = av_parser_init(codec->id);
		c = avcodec_alloc_context3(codec);
		frame = av_frame_alloc();
		if (avcodec_open2(c, codec, NULL) < 0) {
			av_parser_close(parser);
			avcodec_free_context(&c);
			av_packet_free(&pkt);
			strm_idx = -1;
			continue;
		}

		w = in_strm->codecpar->width;
		h = in_strm->codecpar->height;
		localVideoBufferDrawer = new unsigned char[w*h*3/2];
		in_buff = new unsigned char[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
		if(in_strm->codecpar->extradata) {
			c->extradata_size = in_strm->codecpar->extradata_size;
			c->extradata = (uint8_t*)av_mallocz(in_strm->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
			if (c->extradata) {
				memcpy(c->extradata, in_strm->codecpar->extradata, in_strm->codecpar->extradata_size);
			} else {
				c->extradata_size = 0;
				uint8_t *data_buff = in_strm->codecpar->extradata;
				int data_size = in_strm->codecpar->extradata_size;
				int ret;
				while (data_size > 0) {
					ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
							data_buff, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
					if (ret < 0) {
						break;
					}

					data_buff += ret;
					data_size -= ret;

					if (pkt->size) {
						ret = avcodec_send_packet(c, pkt);
						if (ret < 0) {
							break;
						}

						while (ret >= 0) {
							ret = avcodec_receive_frame(c, frame);
							if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
								break;
							else if (ret < 0) {
								break;
							}

							int justifiedHeight;
							ret = RenderAVFrame(frame, justifiedHeight);
							if (ret < 0)
								break;
						} // end of while ret >= 0
					} // end of pkt->size > 0
				} // end of while data_size > 0
			}
		} // end of if

		std::cout << "Got frame with resolution = " << w << 'x' << h << std::endl;

		break;
	}

	// main loop
	while (1) {
		glutMainLoopEvent(); // detect glut event
		ret = av_read_frame(input, pkt);

		if (ret != 0) {
			if (ret == AVERROR(EAGAIN)) {//read again (cam don't return packet)
				av_usleep(10000);
				std::cout << "Read again" << std::endl;
				continue;
			} else {
				break;
			}
		}
		else { // got package

			if (pkt->stream_index != strm_idx) {// no map stream
				av_packet_unref(pkt);
				continue;
			}

			memcpy(in_buff, pkt->data, pkt->size);
			uint8_t *data_buff = in_buff;
			int data_size = pkt->size;
			av_packet_unref(pkt);

			pkt->data = 0;
			pkt->size = 0;
			pkt->buf  = 0;
			while (data_size > 0) {
				ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
						data_buff, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
				if (ret < 0) {
					break;
				}

				data_buff += ret;
				data_size -= ret;
				if (pkt->size) {
					ret = avcodec_send_packet(c, pkt);
					if (ret < 0) {
						continue;
					}

					while (ret >= 0) {
						ret = avcodec_receive_frame(c, frame);
						if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
							break;
						else if (ret < 0) {
							break;
						}

						int justifiedHeight;
						ret = RenderAVFrame(frame, justifiedHeight);
						if (ret < 0)
							break;

						DrawGLScene();
					} // end of while ret >= 0
				} // end of pkt->size > 0
			}
		}
	}

	std::cout << "Program exit now" << std::endl;
	glutDestroyWindow(window);
	return 0;
}
