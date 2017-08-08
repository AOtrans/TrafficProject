#include "getConnectedDomain.h"

void getConnectedDomain::ronny_fillRunVectors(const Mat& bwImage, int& NumberOfRuns, vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun)
{
	int rows = bwImage.rows;
	int cols = bwImage.cols;
	for (int i = 0; i < rows; i++)
	{
		const uchar* rowData = bwImage.ptr<uchar>(i);

		if (rowData[0] != 0)
		{
			NumberOfRuns++;
			stRun.push_back(0);
			rowRun.push_back(i);
		}
		for (int j = 1; j < cols; j++)
		{
			if (rowData[j - 1] == 0 && rowData[j] != 0)
			{
				NumberOfRuns++;
				stRun.push_back(j);
				rowRun.push_back(i);
			}
			else if (rowData[j - 1] != 0 && rowData[j] == 0)
			{
				enRun.push_back(j);                         //问题，是push j-1 还是 j
			}
		}
		if (rowData[bwImage.cols - 1] != 0)
		{
			enRun.push_back(bwImage.cols - 1);
		}
	}
}
void getConnectedDomain::ronny_firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns,vector<int>& runLabels, vector<pair<int, int>>& equivalences, int offset)
{
	runLabels.assign(NumberOfRuns, 0);
	int idxLabel = 1;
	int curRowIdx = 0;
	int firstRunOnCur = 0;
	int firstRunOnPre = 0;
	int lastRunOnPre = -1;
	for (int i = 0; i < NumberOfRuns; i++)
	{
		if (rowRun[i] != curRowIdx)
		{
			curRowIdx = rowRun[i];
			firstRunOnPre = firstRunOnCur;
			lastRunOnPre = i - 1;
			firstRunOnCur = i;

		}
		for (int j = firstRunOnPre; j <= lastRunOnPre; j++)
		{
			if (stRun[i] <= enRun[j] + offset && enRun[i] >= stRun[j] - offset && rowRun[i] == rowRun[j] + 1)
			{
				if (runLabels[i] == 0) // 没有被标号过
					runLabels[i] = runLabels[j];
				else if (runLabels[i] != runLabels[j])// 已经被标号             
					equivalences.push_back(make_pair(runLabels[i], runLabels[j])); // 保存等价对
			}
		}
		if (runLabels[i] == 0) // 没有与前一列的任何run重合
		{
			runLabels[i] = idxLabel++;
		}

	}
}
void getConnectedDomain::ronny_replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>&equivalence)
{
	int maxLabel = *max_element(runLabels.begin(), runLabels.end());
	vector<vector<bool>> eqTab(maxLabel, vector<bool>(maxLabel, false));
	vector<pair<int, int>>::iterator vecPairIt = equivalence.begin();
	while (vecPairIt != equivalence.end())
	{
		eqTab[vecPairIt->first - 1][vecPairIt->second - 1] = true;
		eqTab[vecPairIt->second - 1][vecPairIt->first - 1] = true;
		vecPairIt++;
	}
	vector<int> labelFlag(maxLabel, 0);
	vector<vector<int>> equaList;
	vector<int> tempList;

	for (int i = 1; i <= maxLabel; i++)
	{
		if (labelFlag[i - 1])
		{
			continue;
		}
		labelFlag[i - 1] = equaList.size() + 1;
		tempList.push_back(i);
		for (vector<int>::size_type j = 0; j < tempList.size(); j++)
		{
			for (vector<bool>::size_type k = 0; k != eqTab[tempList[j] - 1].size(); k++)
			{
				if (eqTab[tempList[j] - 1][k] && !labelFlag[k])
				{
					tempList.push_back(k + 1);
					labelFlag[k] = equaList.size() + 1;
				}
			}
		}
		equaList.push_back(tempList);
		tempList.clear();
	}

	for (vector<int>::size_type i = 0; i != runLabels.size(); i++)
	{
		runLabels[i] = labelFlag[runLabels[i] - 1];
	}
}
bool getConnectedDomain::findDomainBaseStrokeBaseOnRonny(Mat &Image,vector<vector<int>> &ableRectangle)
{
	Mat image = Image.clone();

	int NumberOfRuns = 0;
	vector<int> stRun;
	vector<int> enRun;
	vector<int> rowRun;
	ronny_fillRunVectors(image,NumberOfRuns,stRun,enRun,rowRun);

	if(NumberOfRuns == 0)
		return false;                            //没有生成连通域，返回false

	vector<int> runLabels; 
	vector<pair<int, int>> equivalences; 
	int offset = 0;

	for(int i = 0;i < NumberOfRuns;i++)
		runLabels.push_back(0);

	ronny_firstPass(stRun,enRun,rowRun, NumberOfRuns,runLabels,equivalences,offset);

	ronny_replaceSameLabel(runLabels,equivalences);

	vector<int>type;
	type.push_back(runLabels[0]);                      //BUG??????????????????????????????????????????????????????
	bool isChange = false;
	for(int i = 0;i < NumberOfRuns;i++)
	{
		isChange = false;
		for(int j = 0;j < type.size();j++)
		{
			if(type[j] == runLabels[i])
			{
				isChange = true;
				break;
			}
		}

		if(!isChange)
			type.push_back(runLabels[i]);
	}

	vector<int> subAbleRectangle;

	for(int i = 0;i < type.size();i++)
	{
		int down = 0,up = 0;
		int left = image.cols,right = 0;
		for(int j = 0;j < NumberOfRuns;j++)
		{
			if(runLabels[j] == type[i])
			{
				down = rowRun[j];
				break;
			}
		}
		for(int j = NumberOfRuns-1;j > -1;j--)
		{
			if(runLabels[j] == type[i])
			{
				up = rowRun[j];
				break;
			}
		}

		for(int j = 0;j < NumberOfRuns;j++)
		{
			if(runLabels[j] == type[i])
			{
				left = (left > stRun[j] ? stRun[j]:left);
				right = (right < enRun[j] ? enRun[j]:right);
			}
		}
		subAbleRectangle.push_back(down);                                                
		subAbleRectangle.push_back(up);
		subAbleRectangle.push_back(left);
		subAbleRectangle.push_back(right);
		ableRectangle.push_back(subAbleRectangle);
		subAbleRectangle.clear();
	}

	return true;
}
