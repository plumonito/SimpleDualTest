#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Device.h"
#include "System.h"
#include "Application.h"
#include "GLES2Util.h"

class TestApp: public System::Application
{
public:
	TestApp() :
		mQuit(false)
	{
	}

private:
	std::unique_ptr<GLES2Util::QuadRenderer> mRenderer;
	cv::VideoCapture mCapture;
	uint mTexId;
	bool mQuit;

	bool init(void)
	{
		LOG_DEBUG(LOG_INFO, "Simple test init\n");

		const char fragmentShader[] =
				"precision mediump float;\n\
		        uniform sampler2D uSourceTex;\n\
		        varying vec2 vTexCoord;\n\
		        void main(void)\n\
		        {\n\
		            gl_FragColor = vec4(1.0,0.0,1.0,1.0);\n\
		        }\n";

		mRenderer.reset(GLES2Util::QuadRenderer::Create(GLES2Util::QuadRenderer::DefaultFragmentShaderSampler2D));

		mCapture.open(0);
		if (!mCapture.isOpened())
			LOG_DEBUG(LOG_INFO, "Camera failed to open");
		else
		{
			LOG_DEBUG(LOG_INFO, "Camera open succesfully");

			cv::Mat m;
			mCapture.read(m);

			glGenTextures(1, &mTexId);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		return true;
	}
	
	bool update() 
	{
		//Warning: update will be called before init
		return mQuit;
	}

	void keyUp(System::Key button)
	{
		mQuit = true;
	}

	void touchUp(int id, int x, int y)
	{
		mQuit = true;
	}

	void draw()
	{
		cv::Mat m;

		//Read image
		mCapture.read(m);

		//Draw dummy rectangle
		float aspect = (float)m.cols / m.rows;
		cv::Point center(m.cols / 2, m.rows / 2);
		int height = 100;
		int width = height*aspect;
		cv::Rect r(center.x - width / 2, center.y - height / 2, width, height);

		cv::rectangle(m, r, cv::Scalar(0, 255, 0));

		//Convert to RGB
		cv::Mat mRGB;
		cv::cvtColor(m, mRGB, cv::COLOR_BGR2RGB);

		//Upload to GPU
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mRGB.cols, mRGB.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, mRGB.data);

		mRenderer->draw(GL_TEXTURE_2D, mTexId);
	}
};

managed_ptr<System::Application> System::Application::AppInit(void)
{
	return new TestApp();
}
