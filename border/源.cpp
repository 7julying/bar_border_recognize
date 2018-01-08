#include "core/core.hpp"
#include "highgui/highgui.hpp"
#include "imgproc/imgproc.hpp"
#include <math.h>
#include "source.h"
#include "zbar.h"

#define PI 3.1415926

using namespace std;
using namespace cv;
using namespace zbar;

int main(int argc, char *argv[])
{
	Mat image, imageGray, imageGuussian, imageEqualize;

	/*----------------------------------------
	1. ��ͼ��
	----------------------------------------*/
	image = imread(argv[1]);
	myImShow("1.ԭͼ��", image,ZIP,1);

	/*----------------------------------------
	2.ת��Ϊ�Ҷ�ͼ
	----------------------------------------*/
	cvtColor(image, imageGray, CV_RGB2GRAY);
	//Ϊ�˷��㣬��ͼƬѹ������������˷�װ
	myImShow("2.�Ҷ�ͼ", imageGray,ZIP,1);

	/*----------------------------------------
	3. ��˹ƽ���˲� 
	----------------------------------------*/
	GaussianBlur(imageGray, imageGuussian, Size(9, 9), 0);
	myImShow("3.��˹ƽ���˲�", imageGuussian,ZIP,1);

	/*----------------------------------------
	4. ˫��ȵ׷�Ѱ����ֵ��ֵ��
	----------------------------------------*/
	//����ֱ��ͼ
	MatND histG = myCalcHist(imageGuussian, 0);
	//˫��ȵ׷�Ѱ����ֵ
	int vally = findThresholdVally(histG);
	printf("��ֵ����ֵ��%d\r\n", vally);
	//��ֵ��
	Mat imageThreshold;
	threshold(imageGray, imageThreshold, vally, 255, CV_THRESH_BINARY);
	myImShow("��ֵ��", imageThreshold,ZIP,1);

	/*----------------------------------------
	5. �ж������뷽����ת
		ʹ��Sobel���ӷֱ����X��Y�����ݶ�
		���������������ݶ����ҵļ��㣬ͳ�Ƴ��仯��Ƶ������
		����������������������ˮƽ��Ϊ�ڰ׽��棬�ݶȱ任Ƶ��
		����ͳ����Ϣ��תͼ��
	----------------------------------------*/
	//���ˮƽ�ʹ�ֱ����Ҷ�ͼ����ݶ�,ʹ��Sobel����
	Mat imageX16S, imageY16S;
	Mat imageSobelX, imageSobelY;
	Mat imageDirection;
	Sobel(imageThreshold, imageX16S, CV_16S, 1, 0, 3, 1, 0, 4);
	Sobel(imageThreshold, imageY16S, CV_16S, 0, 1, 3, 1, 0, 4);
	
	//����ÿ�����ص��ݶȷ���ͳ�Ʒ�ֵ����0�⣩
	findDirection(imageX16S, imageY16S, imageDirection);
	int max = hist16S(imageDirection);
	printf("���ֵλ�ã�%d\r\n", max);

	//�ݶ�ͼ����ʾ
	//sobel�����ÿ��������short���ͣ���Ҫת��Ϊ�޷�����
	convertScaleAbs(imageX16S, imageSobelX, 1, 0);
	convertScaleAbs(imageY16S, imageSobelY, 1, 0);
	convertScaleAbs(imageDirection, imageDirection, 1, 0);
	myImShow("X����", imageSobelX,ZIP,1);
	myImShow("Y����", imageSobelY,ZIP,1);
	myImShow("5. ����", imageDirection,ZIP,1);

	//��תͼ��
	double angle = max / 255.0 * 90;
	//������ת��Ĵ�С��������ת
	cv::Point2f center(image.cols / 2.0f, image.rows / 2.0f);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1);
	cv::Rect bbox = cv::RotatedRect(center, image.size(), angle).boundingRect();
	//������ת
	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;
	
	Mat imageRotate;
	Mat imageGrayRotate;
	Mat imageThresholdRotate;
	Scalar borderColor = Scalar(255,255,255);
	//��ΪĬ����ת������ɫ������ɫ��ת
	imageThreshold = 255 - imageThreshold;
	warpAffine(image, imageRotate, rot, bbox.size(), INTER_LINEAR, BORDER_CONSTANT,borderColor);
	warpAffine(imageThreshold, imageThresholdRotate, rot, bbox.size());
	warpAffine(imageGray, imageGrayRotate, rot, bbox.size());
	myImShow("5.��תͼ��", imageGrayRotate, ZIP, 1);

	/*----------------------------------------
	6. ����ͨ����
		contours��¼�����ҵ�������
		rectVector�м�¼����������
		�ж�contours�е�Ԫ���Ƿ���rectVector��
		һ��������Ķ�����ֲ��ظ�����
	----------------------------------------*/
	//������ͨ����
	vector<vector<Point>> contours;
	vector<Vec4i> hiera;
	findContours(imageThresholdRotate, contours, hiera, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	printf("counters:%d\r\n", contours.size());
	int imageWidth = imageRotate.cols;
	int j = 0;
	Vector<Rect> rectVector;
	//����ͨ�������
	for (int i = 0;i<contours.size();i++)
	{
		//�򵥹���
		//�����Ȳ�����ͼ��10%��������ǿ��3�����ϼ������Σ������4������
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.width < imageWidth / 10)
		{
			if (rect.width * 3 < rect.height && rect.width > 4)
			{
				//�ж��Ƿ����Ѿ��ҵ�����������
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
						//��������ǿ�ʶ���������
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