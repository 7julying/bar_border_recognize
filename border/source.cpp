#include "source.h"

//ƽ��ֵ���Ҷ�ֵ����ֵ
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

bool IsDimodal(double HistGram[])       // ���ֱ��ͼ�Ƿ�Ϊ˫���  
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
//isShow	-0 ������
//			-1 ����
MatND myCalcHist(Mat imageGray, int isShow)
{
	//����ֱ��ͼ
	int channels = 0;
	MatND dstHist;
	int histSize[] = { 256 };
	float midRanges[] = { 0,256 };
	const float *ranges[] = { midRanges };
	calcHist(&imageGray, 1, &channels, Mat(), dstHist, 1, histSize, ranges, true, false);
	//images(&imageGray):����ͼ��ָ�룬���Զ����Ҫ�����ͬ�����
	//nimages(1):����ͼ�����
	//channels(&channels):����ֱ��ͼ��channels������
	//mask(Mat()):���룬���mask��Ϊ�գ�������һ��8λ��CV_8U������
	//hist(hstHist):ÿһά��ֱ��ͼ��Ԫ�ظ���
	//dims��1����ֱ��ͼά��
	//histSize��histSize)��ֱ��ͼÿάԪ�ظ���
	//ranges��ranges����ֱ��ͼÿά��Χ
	//uniform��true����ture˵����Ҫ�����ֱ��ͼ��ÿһά�������ķ�Χ�ͳߴ�ȡ��ֵ
	//accumulate��false�����Ƿ��hist���㣬�����������ֱ��ͼ�ۼ�

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
//type��0-�Ҷ�ͼ��
//		1-��ɫͼ��
int findDirection(Mat &inputImageX, Mat &inputImageY, Mat &outputImage, int type)
{
	if (inputImageX.cols != inputImageY.cols)
		return -1;
	if (inputImageX.rows != inputImageY.rows)
		return -1;

	if (type)
		outputImage = Mat::zeros(inputImageX.rows, inputImageX.cols, CV_8UC3);
	else
		outputImage.create(inputImageX.size(), inputImageX.type());

	uchar* dataX = inputImageX.ptr<unsigned char>(0);
	uchar* dataY = inputImageY.ptr<unsigned char>(0);
	uchar* data = outputImage.ptr<unsigned char>(0);

	int i, j;
	for (i = 0; i < inputImageX.rows;i++)
	{
		for (j = 0; j < inputImageX.cols;j++)
		{
			if (*dataY < 50 && *dataX < 50)
			{
				if (type)
				{
					outputImage.at<Vec3b>(i, j)[0] = 0;
					outputImage.at<Vec3b>(i, j)[1] = 0;
				}
				else
				{
					*data = 0;
				}
			}
			else if (*dataX == 0)
			{
				if (type)
				{
					outputImage.at<Vec3b>(i, j)[0] = 255;
					outputImage.at<Vec3b>(i, j)[1] = 255;
				}
				else
				{
					*data = 255;
				}
			}
			else
			{
				if (type)
				{
					outputImage.at<Vec3b>(i, j)[0] = 255;
					outputImage.at<Vec3b>(i, j)[1] = atan(*dataY / *dataX) / PI * 2 * 254 + 1;
				}
				else
				{
					*data = atan(*dataY / *dataX) / PI * 2 * 254 + 1;
				}

			}
			if (type)
			{
				outputImage.at<Vec3b>(i, j)[2] = 0;
				if (outputImage.at<Vec3b>(i, j)[1] == 1)
					outputImage.at<Vec3b>(i, j)[2] = 255;
			}
			data++;
			dataX++;
			dataY++;
		}

	}

	return 0;
}

//��������
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
void myImShow(char *imageName, Mat &image, int isZip, int isSave)
{
	Mat imagZip;
	if (isZip)
	{
		resize(image, imagZip, Size(), 0.2, 0.2);
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


//��֤�Ƿ��Ƿ���
bool findBloak(Mat & image, Rect & rect)
{
	int x = rect.tl().x;
	int y = rect.tl().y + rect.height / 2;

	int y0_0, y0_1, y0_2, y0_3;
	int y1_1, y1_2;
	int y2_1, y2_2;

	int i = 0;
	int width[10];

	if (x < rect.width || x > image.cols - rect.width)
		return false;

	//x -= rect.width / 2;		//���Ʋ��֣���֤��⵽����
	int edge_last = 0;
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
			else if ((y1_sign < 0) && (y1_1 > 0))
			{
				edge_cur = x;
				width[i] = edge_cur - edge_last;
				edge_last = edge_cur;
				y1_sign = y1_1;
				i++;
			}
			else if ((y1_sign > 0) && (y1_1 < 0))
			{
				edge_last = x;
				y1_sign = y1_1;
			}
		}


		x++;
		if ((x - edge_last > rect.width * 3) || (x == image.cols))
		{
			return false;
		}
		if (i > 5)
			return true;
	}
	

	return false;
}
