#include "core/core.hpp"
#include "highgui/highgui.hpp"
#include "imgproc/imgproc.hpp"
#include <math.h>
#include "source.h"
#include "zbar.h"
#include <iostream>

#define PI 3.1415926

using namespace std;
using namespace cv;
using namespace zbar;






int main(int argc, char *argv[])
{
	Mat image, imageGray, imageGuussian, imageEqualize;
	image = imread(argv[1]);

	//1. ԭͼ���С�������������Ч��
	//myImShow("1.ԭͼ��", image,ZIP,0);

	//2. ת��Ϊ�Ҷ�ͼ
	cvtColor(image, imageGray, CV_RGB2GRAY);
	myImShow("2.�Ҷ�ͼ", imageGray,ZIP,0);

	//3. ��˹ƽ���˲�
	GaussianBlur(imageGray, imageGuussian, Size(9, 9), 0);
	myImShow("3.��˹ƽ���˲�", imageGuussian,ZIP,0);

	//4. ˫��ȵ׶�ֵ��
	MatND histG = myCalcHist(imageGuussian, 0);
	int vally = findThresholdVally(histG);
	printf("��ֵ����ֵ��%d\r\n", vally);
	Mat imageThreshold;
	threshold(imageGray, imageThreshold, vally, 255, CV_THRESH_BINARY);
	//eraseBackground(imageGuussian, imageThreshold, vally);
	myImShow("��ֵ��", imageThreshold,ZIP,0);


	//5.���ˮƽ�ʹ�ֱ����Ҷ�ͼ����ݶȲ�,ʹ��Sobel����
	Mat imageX16S, imageY16S;
	Mat imageSobelX, imageSobelY;
	Mat imageDirection;
	Sobel(imageThreshold, imageX16S, CV_16S, 1, 0, 3, 1, 0, 4);
	Sobel(imageThreshold, imageY16S, CV_16S, 0, 1, 3, 1, 0, 4);
	convertScaleAbs(imageX16S, imageSobelX, 1, 0);
	convertScaleAbs(imageY16S, imageSobelY, 1, 0);
	findDirection(imageSobelX, imageSobelY, imageDirection,0);

	myImShow("X����", imageSobelX,ZIP,0);
	myImShow("Y����", imageSobelY,ZIP,0);
	myImShow("����", imageDirection,ZIP,1);

	//����ֱ��ͼ
	MatND hist = myCalcHist(imageDirection, 0);

	double histMaxValue;
	Point histMaxLoc;
	hist.at<float>(0) = 0;
	minMaxLoc(hist, 0, &histMaxValue, 0, &histMaxLoc);
	printf("���ֵ��%f,λ�ã�%d\r\n", histMaxValue, histMaxLoc.y);

	//��תͼ��
	double angle = histMaxLoc.y / 255.0 * 90;
	cv::Point2f center(image.cols / 2.0f, image.rows / 2.0f);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1);
	cv::Rect bbox = cv::RotatedRect(center, image.size(), angle).boundingRect();

	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;
	
	Mat imageRotate;
	Mat imageGrayRotate;
	Mat imageThresholdRotate;
	Scalar borderColor = Scalar(255,255,255);
	warpAffine(image, imageRotate, rot, bbox.size(), INTER_LINEAR, BORDER_CONSTANT,borderColor);
	warpAffine(imageThreshold, imageThresholdRotate, rot, bbox.size());
	warpAffine(imageGray, imageGrayRotate, rot, bbox.size());
	myImShow("5.��תͼ��", imageThresholdRotate, ZIP, 1);

	//6. ����ͨ����
	vector<vector<Point>> contours;
	vector<Vec4i> hiera;
	Mat imageInvers = 255 - imageThresholdRotate;
	findContours(imageInvers, contours, hiera, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	printf("counters:%d\r\n", contours.size());
	int imageWidth = imageRotate.cols;
	int j = 0;
	Vector<Rect> rectVector;
	for (int i = 0;i<contours.size();i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.width < imageWidth / 10)
		{
			if (rect.width * 3 < rect.height && rect.width > 4)
			{
				int xCurent = rect.tl().x;
				int yCenter = rect.tl().y + rect.height;
				int rectI;
				for (rectI = 0; rectI < rectVector.size();rectI++)
				{
					Rect rectT = rectVector[rectI];
					if ((xCurent > rectT.tl().x) && (xCurent < rectT.tl().x + rectT.width) && (yCenter > rectT.tl().y) && (yCenter < rectT.tl().y + rectT.height))
					{
						break;
					}
				}

				if (rectI == rectVector.size())
				{
					Rect rectTem;

					//�������
					if (findBloak(imageGrayRotate, rect, rectTem))
					{
						//������ʶ��
						ImageScanner scanner;
						scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
						Mat imageCut = Mat(imageGrayRotate, rectTem);
						Mat imageCopy = imageCut.clone();
						uchar *raw = (uchar *)imageCopy.data;
						Image imageZbar(imageCopy.cols, imageCopy.rows, "Y800", raw, imageCopy.cols * imageCopy.rows);
						scanner.scan(imageZbar);		//ɨ��������
						Image::SymbolIterator sybmol = imageZbar.symbol_begin();
						if (imageZbar.symbol_begin() == imageZbar.symbol_end())
						{
							continue;
						}
						rectVector.push_back(rectTem);
						printf("height:%d;width:%d\r\n", rect.height, rect.width);
						printf("x:%d,y:%d\r\n", xCurent, yCenter);
						rectangle(imageRotate, rectTem, Scalar(255), 2);
					}
				}
				
			}
			
		}
	}
	namedWindow("6. �ҳ���ά���������",0);
	myImShow("6. �ҳ���ά���������", imageRotate,ZIP,1);


	

	waitKey();
	return 0;
}