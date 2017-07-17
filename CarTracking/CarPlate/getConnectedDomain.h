#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class getConnectedDomain                       //得到连通域类
{
private:
	void ronny_fillRunVectors(const Mat& bwImage, int& NumberOfRuns, vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun);
	void ronny_firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns,vector<int>& runLabels, vector<pair<int, int>>& equivalences, int offset);
	void ronny_replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>&equivalence);
public:
	bool findDomainBaseStrokeBaseOnRonny(Mat &Image,vector<vector<int>> &ableRectangle);
};
