#include "source.h"

//ƽ��ֵ���Ҷ�ֵ����ֵ
//������	hist��ֱ��ͼ������
//����ֵ��	�ҶȾ�ֵ
int findThresholdAverage(MatND hist)
{
	double histMaxValue;
	Point histMaxLoc;
	minMaxLoc(hist, 0, &histMaxValue, 0, &histMaxLoc);

	double avr = 0;
	double sum = 0;
	for (int i = 0;i < 255;i++)
	{
		sum += hist.at<float>(i);
		avr += (double)hist.at<float>(i) * i;
	}

	return (int)(avr / sum);
}

//���ֱ��ͼ�Ƿ�Ϊ˫���  
//������	HistGram[] ֱ��ͼ����
//����ֵ��	�Ƿ�Ϊ˫��
bool IsDimodal(double HistGram[])       
{
	// ��ֱ��ͼ�ķ���м�����ֻ�з���λ2��Ϊ˫��   
	int Count = 0;
	for (int Y = 1; Y < 255; Y++)
	{
		if (HistGram[Y - 1] < HistGram[Y] && HistGram[Y + 1] < HistGram[Y])
		{
			Count++;
			if (Count > 2) return false;
		}
	}
	if (Count == 2)
		return true;
	else
		return false;
}

//�ȵ���Сֵ��ֵ����ֵ
//������	hist ֱ��ͼ
//����ֵ��	�ȵ׻Ҷ�ֵ
int findThresholdVally(MatND hist)
{
	int Y, Iter = 0;
	double HistGramC[256];           // ���ھ������⣬һ��Ҫ�ø���������������ò�����ȷ�Ľ��  
	double HistGramCC[256];          // ���ֵ�Ĺ��̻��ƻ�ǰ������ݣ������Ҫ��������  
	for (Y = 0; Y < 256; Y++)
	{
		HistGramC[Y] = hist.at<float>(Y);
		HistGramCC[Y] = hist.at<float>(Y);
	}

	// ͨ���������ֵ��ƽ��ֱ��ͼ  
	while (IsDimodal(HistGramCC) == false)                                        // �ж��Ƿ��Ѿ���˫���ͼ����        
	{
		HistGramCC[0] = (HistGramC[0] + HistGramC[0] + HistGramC[1]) / 3;                 // ��һ��  
		for (Y = 1; Y < 255; Y++)
			HistGramCC[Y] = (HistGramC[Y - 1] + HistGramC[Y] + HistGramC[Y + 1]) / 3;     // �м�ĵ�  
		HistGramCC[255] = (HistGramC[254] + HistGramC[255] + HistGramC[255]) / 3;         // ���һ�� 
		memcpy(HistGramC, HistGramCC, sizeof(HistGramCC));
		Iter++;
		if (Iter >= 1000)
			return -1;                                                   // ֱ��ͼ�޷�ƽ��Ϊ˫��ģ����ش������  
	}
	// ��ֵ��Ϊ����֮�����Сֵ   
	bool Peakfound = false;
	for (Y = 1; Y < 255; Y++)
	{
		if (HistGramCC[Y - 1] < HistGramCC[Y] && HistGramCC[Y + 1] < HistGramCC[Y]) Peakfound = true;
		if (Peakfound == true && HistGramCC[Y - 1] >= HistGramCC[Y] && HistGramCC[Y + 1] >= HistGramCC[Y])
			return Y - 1;
	}
	return -1;
}




//����ֱ��ͼ
//������	imageGray �Ҷ�ͼ��
//������	isShow	-0 ������
//					-1 ����
//����ֵ��	�Ҷ�ֱ��ͼ����
MatND myCalcHist(Mat imageGray, int isShow)
{
	//����ֱ��ͼ
	int channels = 0;
	MatND dstHist;
	int histSize[] = { 256 };
	float midRanges[] = { 0,255 };
	const float *ranges[] = { midRanges };
	calcHist(&imageGray, 1, &channels, Mat(), dstHist, 1, histSize, ranges, true, false);

	if (isShow)
	{
		//����ֱ��ͼ,�����ȴ���һ���ڵ׵�ͼ��Ϊ�˿�����ʾ��ɫ�����Ըû���ͼ����һ��8λ��3ͨ��ͼ��    
		Mat drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
		//�κ�һ��ͼ���ĳ�����ص��ܸ����п��ܻ�ܶ࣬���������������ͼ��ĳߴ磬  
		//������Ҫ�ȶԸ������з�Χ�����ƣ���minMaxLoc�������õ�����ֱ��ͼ������ص�������    
		double g_dHistMaxValue;
		minMaxLoc(dstHist, 0, &g_dHistMaxValue, 0, 0);
		//�����صĸ������ϵ�ͼ������Χ��    
		for (int i = 1; i < 256; i++)
		{
			int value = cvRound(dstHist.at<float>(i) * 256 * 0.9 / g_dHistMaxValue);
			line(drawImage, Point(i, drawImage.rows - 1), Point(i, drawImage.rows - 1 - value), Scalar(0, 0, 255));
		}
		line(drawImage, Point(0, drawImage.rows - 1), Point(0, drawImage.rows - 1 - 0), Scalar(0, 0, 255));
		imshow("hist", drawImage);
	}


	return dstHist;
}

//�����ݶ���෽��
//������	inputImageX x�����ݶ�ͼ��
//������	inputImageY y�����ݶ�ͼ��
//������	outputImage ������ͼ��
//����ֵ��	0  - ����
//			-1 - �쳣
int findDirection(Mat &inputImageX, Mat &inputImageY, Mat &outputImage)
{
	if (inputImageX.cols != inputImageY.cols)
		return -1;
	if (inputImageX.rows != inputImageY.rows)
		return -1;

	outputImage.create(inputImageX.size(), inputImageX.type());

	short* dataX = inputImageX.ptr<short>(0);
	short* dataY = inputImageY.ptr<short>(0);
	short* data = outputImage.ptr<short>(0);

	int i, j;
	for (i = 0; i < inputImageX.rows;i++)
	{
		for (j = 0; j < inputImageX.cols;j++)
		{
			if (*dataY < 20 && *dataY > -20 && *dataX > -20 && *dataX < 20)
			{
				//�ݶȱ仯��С���޳�
				*data = 0;
			}
			else if (*dataX == 0)
			{
				if (*dataY != 0)
				{
					*data = 255;
				}
				else
				{	
					*data = 0;
				}
			}
			else
			{				
				*data = atan((float)*dataY / (float)*dataX) / PI * 2 * 254;
				//����������/���������ݶȶ���0�����ݣ�����0��
				//���С��1��ȡ��Ϊ0����Ϊ1
				if (*data == 0)
					(*data)++;
			}
			data++;
			dataX++;
			dataY++;
		}

	}

	return 0;
}

//��������
//������Ϊȫ��0����������
//������	inputImage ����ͼ��
//������	outputImage ���ͼ��
//������	threshold ��ֵ
//����ֵ��	0 - ����
int eraseBackground(Mat &inputImage, Mat &outputImage, int threshold)
{
	outputImage.create(inputImage.size(), inputImage.type());

	uchar* dataIn = inputImage.ptr<unsigned char>(0);
	uchar* dataOut = outputImage.ptr<unsigned char>(0);

	for (int i = 0;i < inputImage.rows;i++)
	{
		for (int j = 0;j < inputImage.cols;j++)
		{
			if (*dataIn < threshold)
				*dataOut = *dataIn;
			else
				*dataOut = 255;

			dataIn++;
			dataOut++;
		}
	}

	return 0;
}

//ͼ����ʾ������ѹ����ʾ�ͱ���
//������	imageName ͼ������
//������	iamge ͼ��
//������	isZip �Ƿ�ѹ����ʾ 1-ѹ�� 0-��ѹ��
//������	isSave �Ƿ񱣴�ͼƬ��������һ����Ӱ�죬ȫ�ֱ��ʱ��棩 1-���� 0-������
void myImShow(char *imageName, Mat &image, int isZip, int isSave)
{
	Mat imagZip;
	if (isZip)
	{
		resize(image, imagZip, Size(), ZIPTIME, ZIPTIME);
	}
	else
	{
		imagZip = image.clone();
	}

	if (isSave)
	{
		char * name = new char[strlen(imageName) + sizeof(char) * 4];
		memcpy(name, imageName, strlen(imageName));
		*(name + strlen(imageName)) = '.';
		*(name + strlen(imageName) + 1) = 'j';
		*(name + strlen(imageName) + 2) = 'p';
		*(name + strlen(imageName) + 3) = 'g';
		*(name + strlen(imageName) + 4) = 0;
		imwrite(name, image);
	}
	imshow(imageName, imagZip);
}


//��֤�Ƿ�������������
//������	image ͼ��
//������	rect ����Ȥ����
//������	rectOut ����������
//����ֵ��	�Ƿ���������
bool findBloak(Mat & image, Rect & rect,Rect & rectOut)
{
	int rectX, rectY, rectWidth, rectHeight;
	int rectEndX;

	int x = rect.tl().x;
	int y = rect.tl().y + rect.height / 2;

	int y0_0, y0_1, y0_2, y0_3;
	int y1_1, y1_2;
	int y2_1, y2_2;

	int i = 0;

	if (x < rect.width || x > image.cols - rect.width)
		return false;

	//x -= rect.width / 2;		//���Ʋ��֣���֤��⵽����

	//x�������ж�
	int edge_last = x;
	int edge_cur = 0;
	int y1_sign = 0;			//һ�׵�����
	while (1)
	{
		//���
		y0_0 = image.at<uchar>(y,x);
		y0_1 = image.at<uchar>(y,x-1 );
		y0_2 = image.at<uchar>(y,x-2);
		y0_3 = image.at<uchar>(y,x-3);

		//һ�׵�
		y1_1 = y0_1 - y0_2;
		y1_2 = y0_2 - y0_3;
		{
			if ((abs(y1_1) < abs(y1_2)) && ((y1_1 >= 0) == (y1_2 >= 0)))
				y1_1 = y1_2;
		}

		//���׵�
		y2_1 = y0_0 - (y0_1 * 2) + y0_2;
		y2_2 = y0_1 - (y0_2 * 2) + y0_3;

		//���׵�Ϊ0�㣬һ�׵�����/��Сֵ�������Ǳ���
		if (!y2_1 || ((y2_1 > 0) ? y2_2 < 0 : y2_2>0))
		{
			if (!y1_sign && y1_1)
			{
				edge_last = edge_cur = x;
				y1_sign = y1_1;
			}
			//�ڿ����
			else if ((y1_sign < 0) && (y1_1 > 0))
			{
				edge_cur = x;
				edge_last = edge_cur;
				y1_sign = y1_1;
				i++;
			}
			//�ڿ�ǰ��
			else if ((y1_sign > 0) && (y1_1 < 0))
			{
				edge_last = x;
				y1_sign = y1_1;
			}
		}


		x++;
		//�ڿ򲻳�������Ȥ����1.5����
		//��ɫ���ֲ���������Ȥ����3����
		if ((y1_sign > 0)?(x - edge_last > rect.width * 3):(x - edge_last > rect.width * 1.5) || (x == image.cols))
		{
			if (i > 9)
			{
				//����9��������������������
				rectEndX = x;
				break;
			}
			else
				return false;
		}
	}

	//x��С�����ж�
	x = rect.tl().x;
	edge_last = x;
	edge_cur = 0;
	y1_sign = 0;			//һ�׵�����
	while (1)
	{
		//���
		y0_0 = image.at<uchar>(y, x);
		y0_1 = image.at<uchar>(y, x - 1);
		y0_2 = image.at<uchar>(y, x - 2);
		y0_3 = image.at<uchar>(y, x - 3);

		//һ�׵�
		y1_1 = y0_1 - y0_2;
		y1_2 = y0_2 - y0_3;
		{
			if ((abs(y1_1) < abs(y1_2)) && ((y1_1 >= 0) == (y1_2 >= 0)))
				y1_1 = y1_2;
		}

		//���׵�
		y2_1 = y0_0 - (y0_1 * 2) + y0_2;
		y2_2 = y0_1 - (y0_2 * 2) + y0_3;

		//���׵�Ϊ0�㣬һ�׵�����/��Сֵ�������Ǳ���
		if (!y2_1 || ((y2_1 > 0) ? y2_2 < 0 : y2_2>0))
		{
			if (!y1_sign && y1_1)
			{
				edge_last = edge_cur = x;
				y1_sign = y1_1;
			}
			//�ڿ�ǰ��
			else if ((y1_sign > 0) && (y1_1 < 0))
			{
				edge_cur = x;
				edge_last = edge_cur;
				y1_sign = y1_1;
			}
			//�ڿ����
			else if ((y1_sign < 0) && (y1_1 > 0))
			{
				edge_last = x;
				y1_sign = y1_1;
			}
		}

		x--;
		if ((y1_sign < 0) ? (edge_last - x > rect.width * 3) : (edge_last - x  > rect.width * 1.5) || (x == 5))
		{
			rectX = x;
			rectY = rect.tl().y;
			rectHeight = rect.height;
			rectWidth = rectEndX - rectX;
			rectOut.height = rectHeight;
			rectOut.width = rectWidth;
			rectOut.x = rectX;
			rectOut.y = rectY;
			return true;
		}
	}
	

	return false;
}

//16λͼ����ֱ��ͼ���ֵ
//�������ݷ�Χ-255 -- +254
//0Ϊ��Ч����
//������	image ����ͼ��
//����ֵ��	ֱ��ͼ���ֵ
int hist16S(Mat &image)
{
	int maxLoc = 0;
	int maxValue = 0;
	double hist[512] = { 0 };

	short *data = image.ptr<short>(0);
	
	for (int i = 0; i < image.rows;i++)
	{
		for (int j = 0;j < image.cols;j++)
		{
			hist[*data + 255]++;
			if (hist[*data + 255] > maxValue && *data != 0)
			{
				maxLoc = *data;
				maxValue = hist[*data + 255];
			}
			data++;
		}
	}
	return maxLoc;
}