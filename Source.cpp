#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
/*
Adaptive image thresholding

Create an set of functions for adaptive thresholding.
Adaptive thresholding should work by analyzing a small neighborhood of the threshold point (for example, 16x16 pixels)
Determine the best threshold - the median of the surrounding pixel values, the average, value chosen as the best value separating the histogram peaks (Otsu method).
The project output should be an algorithm, demonstration and description the properties of various approaches.

Guillermo Leiro Arroyo		--Xleiro00--

OpenCV version == 4.5.1_vc15t
*/

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


// Declarations
int initial_threshold(cv::Mat src_gray);
bool load_image(std::string img_path, cv::Mat& src_gray);
void predefined_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int threshold);
void global_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int threshold);
void global_mean_thresholding(cv::Mat src_gray, cv::Mat& threshold_image);
void global_iterative_mean_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int iterations);
void iterative_mean_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int initial_threshold, int iterations);
void adaptative_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int initial_threshold, int iterations, int local_size);
void otsu_adaptative_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int local_size);
void create_histogram(cv::Mat src_gray, int histogram[]);
void otsu_method(cv::Mat src_gray, cv::Mat& threshold_image, const int histogram[]);
void median_adaptative_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int local_size);

int main(int argc, char* argv[]) {

	// variable declaration
	bool status;
	cv::Mat src_gray;
	// you can add image path to the variable down bellow for testing
	std::string img_path = "";
	std::string method = "";
	

	// checking for arguments
	if (argc == 2) {
		img_path = std::string(argv[1]);
	}else if (argc == 3) {
		img_path = std::string(argv[1]);
		method = std::string(argv[2]);
	}else {
		std::cout << "wrong number of arguments." << std::endl;
	}

	// Image loading and grayscale conversion to CV_8UC1 (uchar)
	status = load_image(img_path, src_gray);
	if (status) {
		cv::Mat threshold_image = cv::Mat::zeros(src_gray.size(), CV_8UC1);


		// executing algorithm 
		if (method.compare("gmean") == 0) {
			global_mean_thresholding(src_gray, threshold_image); // global based on the mean
		}
		else if (method.compare("global") == 0) {
			global_thresholding(src_gray, threshold_image, 127); // global based on a threshold
		}
		else if (method.compare("giter") == 0) {
			global_iterative_mean_thresholding(src_gray, threshold_image, 10);// global with mean iterations
		}
		else if (method.compare("amean") == 0) {
			adaptative_thresholding(src_gray, threshold_image, initial_threshold(src_gray), 10, 24); // adaptative based on mean and with iterations
		}
		else if (method.compare("otsu") == 0) {
			otsu_adaptative_thresholding(src_gray, threshold_image, 32); // adaptative with otsu method
		}
		else if (method.compare("amedian") == 0) {
			median_adaptative_thresholding(src_gray, threshold_image, 16); // median adaptative thresholding with out iterations
		}
		else {
			std::cout << "No method inputed" << std::endl;
		}


		/*===================================area for testing and experimenting=========================================*/
		//global_thresholding(src_gray, threshold_image, 90);
		//global_mean_thresholding(src_gray, threshold_image);
		//global_thresholding(src_gray, threshold_image, initial_threshold(src_gray));
		//global_iterative_mean_thresholding(src_gray, threshold_image, 4);
		//adaptative_thresholding(src_gray, threshold_image, initial_threshold(src_gray), 10, 2);
		otsu_adaptative_thresholding(src_gray, threshold_image, 32);
		



		cv::imwrite("output.png", threshold_image);
		//std::cout << img_path << std::endl;
	}
}

//////////////////////////////////////////// global thresholding ///////////////////////////////////////////////////////////
// global thresholding with a predefined threshold
void global_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int threshold) {
	// Global thresholding with threshold t= 127
	int t = threshold;
	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<uchar>(i, j) >= 0) {
				if (src_gray.at<uchar>(i, j) < t) {
					threshold_image.at<uchar>(i, j) = 0;
				}
				else {
					threshold_image.at<uchar>(i, j) = 255;
				}
			}
		}
	}

}

// global thresholding with the mean
void global_mean_thresholding(cv::Mat src_gray, cv::Mat& threshold_image) {
	// Global thresholding with threshold t= 127
	int t = 0;
	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			t = t + src_gray.at<uchar>(i, j);
		}
	}
	t = t / (src_gray.rows * src_gray.cols);

	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<uchar>(i, j) < t) {
				threshold_image.at<uchar>(i, j) = 0;
			}
			else {
				threshold_image.at<uchar>(i, j) = 255;
			}
		}
	}
}

// global iterative thresholding with the X iterations
void global_iterative_mean_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int iterations) {
	// Global,iterative,mean,thresholding with threshold t= 127
	int g1, g2, count_1, count_2;
	int m1, m2;
	int t = initial_threshold(src_gray);
	for (int n = 0; n < iterations; n++) {
		g1 = 0;
		g2 = 0;
		count_1 = 0;
		count_2 = 0;
		for (int i = 0; i < src_gray.rows; i++) {
			for (int j = 0; j < src_gray.cols; j++) {
				if (src_gray.at<uchar>(i, j) >= 0) {
					if (src_gray.at<uchar>(i, j) < t) {
						g1 = g1 + 1;
						count_1 = count_1 + src_gray.at<uchar>(i, j);
					}
					else {
						g2 = g2 + 1;
						count_2 = count_2 + src_gray.at<uchar>(i, j);
					}
				}
			}
		}
		t = 0.5 * ((count_1 / g1) + (count_2 / g2));
	}

	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<uchar>(i, j) >= 0) {
				if (src_gray.at<uchar>(i, j) < t) {
					threshold_image.at<uchar>(i, j) = 0;
				}
				else {
					threshold_image.at<uchar>(i, j) = 255;
				}
			}
		}
	}
}


//////////////////////////////////////////// median ///////////////////////////////////////////////////////////
void median_adaptative_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int local_size) {

	int histogram[255];
	int N = 0;
	int t = 0;
	int aux;


	cv::Mat local = cv::Mat::zeros(local_size, local_size, CV_32SC1);
	cv::Mat thresholding_local = cv::Mat::zeros(local_size, local_size, CV_32SC1);

	for (int i = 0; i < src_gray.rows + local_size; i = i + local_size) {
		for (int j = 0; j < src_gray.cols + local_size; j = j + local_size) {
			for (int x = 0; x < local_size; x++) {
				for (int y = 0; y < local_size; y++) {
					if (i + x < src_gray.rows && j + y < src_gray.cols) {
						local.at<int>(x, y) = src_gray.at<uchar>(i + x, j + y);
					}
					else {
						local.at<int>(x, y) = -1;
					}
				}
			}

			///// median stuff /////
			create_histogram(local, histogram);
			N = 0;
			for (int i = 0; i < 255; i++) {
				N = N + histogram[i];
			}
			aux = 0;
			for (int i = 0; i < 255; i++) {
				if (aux >= N / 2) {
					t = i;
					break;
				}
				else {
					aux = aux + histogram[i];
				}
			}

			predefined_thresholding(local, thresholding_local, t);
			////////////////////////


			for (int x = 0; x < local_size; x++) {
				for (int y = 0; y < local_size; y++) {
					if (i + x < src_gray.rows && j + y < src_gray.cols) {
						threshold_image.at<uchar>(i + x, j + y) = thresholding_local.at<int>(x, y);
					}
				}
			}
		}
	}
}

/////////////////////////////////////////// Otsu method ///////////////////////////////////////////////////////
// adaptative mean thresholding
void otsu_adaptative_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int local_size) {
	
	int histogram[255];

	//create_histogram(src_gray, histogram);

	// print histogram
	/*for (int i = 0; i < 255; i++) {
		std::cout << histogram[i] << std::endl;
	}*/
	//std::cout << histogram << std::endl;

	cv::Mat local = cv::Mat::zeros(local_size, local_size, CV_32SC1);
	cv::Mat thresholding_local = cv::Mat::zeros(local_size, local_size, CV_32SC1);

	for (int i = 0; i < src_gray.rows + local_size; i = i + local_size) {
		for (int j = 0; j < src_gray.cols + local_size; j = j + local_size) {
			//std::cout << i << "---" << j << std::endl;
			for (int x = 0; x < local_size; x++) {
				for (int y = 0; y < local_size; y++) {
					if (i + x < src_gray.rows && j + y < src_gray.cols) {
						local.at<int>(x, y) = src_gray.at<uchar>(i + x, j + y);
					}
					else {
						local.at<int>(x, y) = -1;
					}
				}
			}

			//std::cout << histogram << std::endl;
			// fuction for thresholding
			create_histogram(local, histogram);
			otsu_method(local, thresholding_local, histogram);


			for (int x = 0; x < local_size; x++) {
				for (int y = 0; y < local_size; y++) {
					if (i + x < src_gray.rows && j + y < src_gray.cols) {
						threshold_image.at<uchar>(i + x, j + y) = thresholding_local.at<int>(x, y);
					}
				}
			}
		}
	}
}

// implementation of Otsu algorith; meant to be called by adaptative_thresholding
void otsu_method(cv::Mat src_gray, cv::Mat& threshold_image, const int histogram[]) {

	// variable declaration
	const int max_intensity = 255;
	int bcv[max_intensity];// between class variance
	int var_max = 0;
	int threshold = 0;
	int sum = 0;
	int sumB = 0;
	int q1 = 0, q2 = 0;
	int u1 = 0, u2 = 0;
	int N = 0;


	for (int i = 0; i < max_intensity; i++) {
		N = N + histogram[i];
		sum = sum + i * histogram[i];
		bcv[i] = 0;
	}


	for (int t = 0; t < max_intensity; t++) {
		q1 = q1 + histogram[t];
		if (q1 != 0 && q1 != N) {
			q2 = N - q1;
			sumB = sumB + t * histogram[t];
			u1 = sumB / q1;
			u2 = (sum - sumB) / q2;

			bcv[t] = q1 * q2 * ((u1 - u2) * (u1 - u2));
		}
	}

	for (int i = 0; i < max_intensity; i++) {
		if (bcv[i] > var_max) {
			threshold = i;
			var_max = bcv[i];
		}
		
	}
	//std::cout << "bcv " << var_max << " //i " << threshold << std::endl;

	//std::cout << "threshold " << threshold << std::endl;

	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<int>(i, j) >= 0) {
				if (src_gray.at<int>(i, j) < threshold) {
					threshold_image.at<int>(i, j) = 0;
				}
				else {
					threshold_image.at<int>(i, j) = 255;
				}
			}
		}
	}

}


//////////////////////////////////////////// mean adaptative ///////////////////////////////////////////////////////////
// adaptative mean thresholding
void adaptative_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int initial_threshold, int iterations, int local_size) {
	int g1, g2, count_1, count_2;
	int m1, m2;
	int t = initial_threshold;

	cv::Mat local = cv::Mat::zeros(local_size, local_size, CV_32SC1);
	cv::Mat thresholding_local = cv::Mat::zeros(local_size, local_size, CV_32SC1);

	for (int i = 0; i < src_gray.rows + local_size; i = i + local_size) {
		for (int j = 0; j < src_gray.cols + local_size; j = j + local_size) {
			//std::cout << i << "---" << j << std::endl;
			for (int x = 0; x < local_size; x++) {
				for (int y = 0; y < local_size; y++) {
					if (i+x < src_gray.rows && j+y < src_gray.cols) {
						local.at<int>(x, y) = src_gray.at<uchar>(i+x, j+y);
					}
					else {
						local.at<int>(x, y) = -1;
					}
					
						
				}

			}

			// fuction for thresholding
			//predefined_thresholding(local, thresholding_local, t);
			iterative_mean_thresholding(local, thresholding_local, t, iterations);
			

			for (int x = 0; x < local_size; x++) {
				for (int y = 0; y < local_size; y++) {
					if (i + x < src_gray.rows && j + y < src_gray.cols) {
						threshold_image.at<uchar>(i + x, j + y) = thresholding_local.at<int>(x, y);
					}

				}

			}


		}

	}
	
}

// iterative thresholding with the X iterations; for been called in the adaptative fuction
void iterative_mean_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int initial_threshold, int iterations) {
	// Global,iterative,mean,thresholding with threshold t= 127
	int g1, g2, count_1, count_2;
	int m1, m2;
	int t = initial_threshold;
	for (int n = 0; n < iterations; n++) {
		g1 = 0;
		g2 = 0;
		count_1 = 0;
		count_2 = 0;
		for (int i = 0; i < src_gray.rows; i++) {
			for (int j = 0; j < src_gray.cols; j++) {
				if (src_gray.at<int>(i, j) >= 0) {
					if (src_gray.at<int>(i, j) < t) {
						g1 = g1 + 1;
						count_1 = count_1 + src_gray.at<int>(i, j);
						//global_t_image.at<uchar>(i, j) = 0;
					}
					else {
						g2 = g2 + 1;
						count_2 = count_2 + src_gray.at<int>(i, j);
						//global_t_image.at<uchar>(i, j) = 255;
					}
				}
			}
		}
		if (g1 == 0 && g2 == 0) {
			t = 0.5 * t;
		}
		else if (g1 == 0) {
			t = 0.5 * (count_2 / g2);
		}
		else if (g2 == 0) {
			t = 0.5 * (count_1 / g1);
		}
		else {
			t = 0.5 * ((count_1 / g1) + (count_2 / g2));
		}
		//t = 0.5 * ((count_1 / g1) + (count_2 / g2));
	}

	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<int>(i, j) >= 0) {
				if (src_gray.at<int>(i, j) < t) {
					threshold_image.at<int>(i, j) = 0;
				}
				else {
					threshold_image.at<int>(i, j) = 255;
				}
			}
		}
	}
}

// thresholding with a predefined threshold; to be called from adaptative fuction
void predefined_thresholding(cv::Mat src_gray, cv::Mat& threshold_image, int threshold) {
	// Global thresholding with threshold t= 127
	int t = 127;
	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<int>(i, j) >= 0) {
				if (src_gray.at<int>(i, j) < t) {
					threshold_image.at<int>(i, j) = 0;
				}
				else {
					threshold_image.at<int>(i, j) = 255;
				}
			}
		}
	}

}


///////////////////////////////////////// other fuctions /////////////////////////////////////////////////////
// creation of image histogram
void create_histogram(cv::Mat src_gray, int histogram[]) {
	// initialize histogram
	for (int i = 0; i < 255; i++) {
		histogram[i] = 0;
	}

	// histogram clasification
	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<int>(i, j) != -1) {
				histogram[src_gray.at<int>(i, j)]++;
			}
		}
	}

	/*int sum = 0;
	for (int i = 0; i < 255; i++) {
		sum = sum + histogram[i];
	}*/
	//std::cout << sum << std::endl;
	//std::cout << src_gray.rows * src_gray.cols << std::endl;

}

// find initial threshold
int initial_threshold(cv::Mat src_gray) {
	int t = 0;
	int max = 0;
	int min = 255;
	for (int i = 0; i < src_gray.rows; i++) {
		for (int j = 0; j < src_gray.cols; j++) {
			if (src_gray.at<uchar>(i, j) > max) {
				max = src_gray.at<uchar>(i, j);
			}
			if (src_gray.at<uchar>(i, j) < min) {
				min = src_gray.at<uchar>(i, j);
			}
		}
	}
	return (max + min) / 2;
}

// load the image into a matrix
bool load_image(std::string img_path, cv::Mat& src_gray) {
	
	//cv::Mat src_gray;

	try {
		cv::Mat src_rgb = cv::imread(img_path);
		cvtColor(src_rgb, src_gray, cv::COLOR_BGR2GRAY);
		return true;
	}
	catch (std::exception e) {
		std::cout << "Failed to load the image: "<< img_path << std::endl;
		return false;
	}
	//std::cout << src_gray << std::endl;
}