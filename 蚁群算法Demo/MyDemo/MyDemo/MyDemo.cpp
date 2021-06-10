#include<iostream>
#include<string>
#include<math.h>
#include<fstream>
#include<vector>
#include<map>
#include<unordered_map>
#include<algorithm>
#include<conio.h>

using namespace std;

#pragma region 变量与常量的定义

//数据文件存放的路径
constexpr auto FILE_PATH = "D:\\智能测控工程化\\蚁群算法\\基于蚁群算法的纯电动公交车的最佳路线设计\\data2.txt";
//城市数量
const int CITY_NUMBER = 19;
//蚂蚁数量（一般为城市数量的1.5倍）
const int ANT_NUMBER = 50;
//最大迭代次数
const int  MAX_ITERATIONS_NUMBER = 150;
//路径数量
const int  PATH_NUMBER = 74;
//启发因子，信息素浓度重要性
const double ALPHA = 1.0;
//启发式重要性
const double BETA = 2.0;
//蚂蚁所携带总的信息素
const double Q = 100.0;
//信息素蒸发系数
const double ROU = 0.5;

//两两城市之间的信息素矩阵
vector<vector<double>> pheromoneMatrix;
//两两城市之间的距离
vector<vector<double>> cityDistance;
//下一代的信息素矩阵
vector<vector<double>> nextPheromoneMatrix;
//启发信息矩阵=1/cityDistance[i][j]
vector<vector<double>> inspMartix;

//关键字：string类型，值：int类型。分别对应保存每次迭代的最优路径和对应的路径距离。
map<string, double>routResult;

//含有充电设施的站点编号
vector<int>rechargeNumber = { 3,4,7,8,12,14,16 };

//定义跳表
//SkipList* p = new SkipList();
#pragma endregion

#pragma region 蚂蚁
class Ant {
public:
	//蚂蚁的当前位置
	int antLoc;
	//蚂蚁的禁忌表，存放已访问过的点
	int taBu[CITY_NUMBER];
	//蚂蚁走过路径的点的集合
	int antPath[PATH_NUMBER];
	//判断蚂蚁是否已经到达终点。True:表示到达，FALSE：表示未到达。
	bool flag;
	
};
//初始化蚁群，大小为ANT_NUMBER
Ant antColony[ANT_NUMBER];
#pragma endregion

#pragma region 城市 
class City {
public:
	//城市编号
	int cityNum;
	//选择每个点的概率
	double cityProb;
};
//初始化城市
City cityArray[CITY_NUMBER];
//线性化选择概率，用于轮盘赌算法
double lineCityProb[CITY_NUMBER];
#pragma endregion 

#pragma region 矩阵初始化功能函数
//res:表示待初始化的矩阵
//temp:表示矩阵初始化的值
void initMatrix(vector<vector<double>> &res, double temp) {
	//定义中间变量v
	vector<double>v;
	for (int i = 0; i < CITY_NUMBER; ++i) {
		//清空v
		vector<double>().swap(v);
		for (int j = 0; j < CITY_NUMBER; ++j) {
			v.push_back(temp);
		}
		res.push_back(v);
	}
}
#pragma endregion

#pragma region 返回指定范围内的随机数
///		返回指定范围内的随机浮点数
///     @param dbLow		范围起始数    
///     @param dbUpper  范围终止数
///     @return					返回的此范围内的随机数 
double randomNumber(double dbLow, double dbUpper)
{
	double dbTemp = rand() / ((double)RAND_MAX + 1.0);
	return dbLow + dbTemp * (dbUpper - dbLow);
}
#pragma endregion

#pragma region 蚁群初始化
void initAntsColony() {
	//初始化禁忌表和行走路线
	for (int i = 0; i < ANT_NUMBER; ++i) {
		for (int j = 0; j < CITY_NUMBER; ++j) {
			//初始化蚁群的禁忌表，设置每只蚂蚁的禁忌表初始化为-1，代表从未访问任何点
			antColony[i].taBu[j] = -1;
		}
		for (int j = 0; j < PATH_NUMBER; ++j) {
			//初始化蚁群的行走路径，设置每只蚂蚁的路径初始化为-1，代表还未经过任何点
			antColony[i].antPath[j] = -1;
		}
	}

	//将蚂蚁放入初始位置，此处设置每只蚂蚁的出发点为0，代表从源点出发
	for (int i = 0; i < ANT_NUMBER; ++i) {
		//设置蚂蚁的当前位置为出发位置0
		antColony[i].antLoc = 0;
		//将当前位置更新到每只蚂蚁的禁忌表中，代表已经访问过此点
		antColony[i].taBu[0] = 0;
		//将当前位置更新到每只蚂蚁的行走路径中，代表路径中已有此点
		antColony[i].antPath[0] = 0;
		//将每只蚂蚁的flag设置为0，代表每只蚂蚁都还未到达终点
		antColony[i].flag = 0;
	}
}
#pragma endregion

#pragma region 初始化城市
void initCity() {
	for (int i = 0; i < CITY_NUMBER; ++i) {
		//初始化城市结构体
		//初始化每个城市的编号为-1
		cityArray[i].cityNum = -1;
		//初始化每个城市的选择概率为0
		cityArray[i].cityProb = 0;
		//初始化线性选择概率为0
		lineCityProb[i] = 0;
	}
}
#pragma endregion

#pragma region 初始化所需的各种矩阵，并读取文件距离数据
void initVariousMatrix() {
	//初始化城市距离矩阵为-1
	initMatrix(cityDistance, -1);

	/*             数据文件的读取                */


	//创建文件流对象
	ifstream CityFile;
	//打开文件
	CityFile.open(FILE_PATH, ios::in);
	//如果文件打开失败，则退出
	if (!CityFile.is_open()) {
		cout << "Open file failure" << endl;
		exit(0);
	}
	//从文件中读取数据到城市距离矩阵之中
	for (int i = 0; i < CITY_NUMBER; ++i) {
		for (int j = 0; j < CITY_NUMBER; ++j) {
			CityFile >> cityDistance[i][j];
		}
	}
	//关闭文件
	CityFile.close();

	//初始化信息素矩阵为0
	 initMatrix(pheromoneMatrix, 0);
	//初始化下一代信息素矩阵为0
	initMatrix(nextPheromoneMatrix, 0);
	//初始化启发信息矩阵为0
	 initMatrix(inspMartix, 0);

	for (int i = 0; i < CITY_NUMBER; ++i) {
		for (int j = 0; j < CITY_NUMBER; ++j) {
			if (cityDistance[i][j] != -1) {                 //如果两个城市之间有距离
				inspMartix[i][j] = 1 / cityDistance[i][j];//启发信息为两两城市距离的倒数
				pheromoneMatrix[i][j] = 1;                //路径上信息素浓度初始值为1
			}
		}
	}
}
#pragma endregion

#pragma region 判断城市j是否在蚂蚁k的禁忌表中
bool ifCityInTabu(int k, int j) {
	for (int i = 0; i < CITY_NUMBER; ++i) {
		//如果城市j,已经在蚂蚁k的禁忌表中，则返回1
		if (j == antColony[k].taBu[i]) {
			return 1;
		}
	}
	//如果不在蚂蚁k的禁忌表中，则返回0
	return 0;
}
#pragma endregion

#pragma region 蚂蚁k从当前城市i选择下一步行进的城市j的概率
double nextCitySelProb(int k, int j) {
	//初始化选择概率公式的分子
	double p = 0;
	//初始化选择概率公式的分母
	double sum = 0;
	//i保存蚂蚁k的当前位置
	int i = antColony[k].antLoc;
	//计算P
	p = pow(pheromoneMatrix[i][j], ALPHA) * pow(inspMartix[i][j], BETA);
	//计算sum
	for (int m = 0; m < CITY_NUMBER; ++m) {
		if (cityDistance[i][j] != -1 && !ifCityInTabu(k, m)) {
			sum += pow(pheromoneMatrix[i][m], ALPHA) * pow(inspMartix[i][m], BETA);
		}
	}
	
	//返回从i--j的选择概率
	return (p / sum);
}
#pragma endregion

#pragma region 更新蚂蚁k的禁忌表信息
void updateAntTabu(int k, int j) {
	//蚂蚁k的当前位置赋值为j
	antColony[k].antLoc = j;
	for (int i = 0; i < CITY_NUMBER; ++i) {
		//如果蚂蚁k还未访问过j，则将蚂蚁k的禁忌表信息添加j
		if (antColony[k].taBu[i] == -1) {
			antColony[k].taBu[i] = j;
			break;
		}
	}
	//同理，将蚂蚁k的行走路径中更新一个j
	for (int i = 0; i < PATH_NUMBER; ++i) {
		if (antColony[k].antPath[i] == -1) {
			antColony[k].antPath[i] = j;
			break;
		}
	}
}
#pragma endregion

#pragma region 轮盘赌选择下一步行进的城市
//蚂蚁k，选择前进的地点j的概率
int nextCitySelect(int k, int f) {

	//记录蚂蚁可行进的城市个数,用于后面的计算
	int c = 0;

	//step 1:计算可行进的各城市的选择概率
	for (int m = 0; m < CITY_NUMBER; ++m) {
		//若城市（i,j）之间有路且j不在蚂蚁k的禁忌表中，则计算概率
		if (cityDistance[antColony[k].antLoc][m] != -1 && !ifCityInTabu(k, m)) {
			//对应的城市编号存入数组中
			cityArray[c].cityNum = m;
			//选择概率存入数组中
			cityArray[c].cityProb = nextCitySelProb(k, m);
			++c;
		}
	}

	//step 2:线性化选择概率
	for (int m = 0; m < c; ++m) {
		for (int n = m; n >= 0; n--) {
			//用于轮盘赌算法,将选择概率线性化累加
			lineCityProb[m] += cityArray[n].cityProb;
		}
	}

	//step 3 产生随机数选择城市
	//产生随机浮点数，范围0-1
	double r = randomNumber(0, 1);
	//选取的目标城市
	int j = 0;
	for (int m = 0; m < CITY_NUMBER; ++m) {
		if (r <= lineCityProb[m]) {
			//将当前选择前进的城市编号赋值给j
			j = cityArray[m].cityNum;
			//将地点j更新至蚂蚁k的禁忌表中，代表已经访问过此点
			updateAntTabu(k, j);
			if (j == f) {
				//若蚂蚁k下一步城市为目的地城市，则修改标志为1
				antColony[k].flag = 1;
			}
			//return j;
			return 0;
		}
	}
	//return f;
	return 0;

}
#pragma endregion

#pragma region 计算路径长度
double getAntLen(Ant ant) {
	//定义路径长度为0
	double len = 0;
	for (int i = 0; i < PATH_NUMBER - 1; ++i) {
		//如果蚂蚁行走的路径中有-1，表示未走过此点，则退出循环
		if (ant.antPath[i] == -1 || ant.antPath[i + 1] == -1)
			break;
		//若走过有路径，则不断累加距离结果
		else
			len += cityDistance[ant.antPath[i]][ant.antPath[i + 1]]; 
	}
	//返回路径长度
	return len;
}
#pragma endregion

#pragma region 计算最优路径对应的蚂蚁编号
int getBestPathAntNumber() {
	//初始化数组d为-1，保存所有蚂蚁的行走路径
	vector<double>d(ANT_NUMBER, -1);
	//假设定义蚂蚁k的路线到达目的地节点最短，即走过的路径为最短路径
	int k = 0;
	//将所有路径距离存入d中
	for (int i = 0; i < ANT_NUMBER; i++)
	{
		d.push_back(getAntLen(antColony[i]));
	}
	//定义指向最小路径的迭代器
	auto min = min_element(d.begin(), d.end());
	//计算此最短路径所对应的下标，此为蚂蚁的编号k
	k = distance(d.begin(), min);
	vector<double>(d).swap(d);
	//返回蚂蚁k
	return k;
}
#pragma endregion

#pragma region 打印最优路径所对应的蚂蚁K的路径和距离
void printBestPath(int k, int f) {

	/*SkipList* p = new SkipList();
	for (auto r : rechargeNumber) {
		p->add(r);
	}*/

	//初始化变量res，存放路线
	string res = "";
	cout << " 最短路径为：";

	for (int i = 0; i < PATH_NUMBER - 1; ++i) {
		//如果蚂蚁k的第i条路径为-1，表示没有走过此点，退出循环
		if (antColony[k].antPath[i] == -1)
			break;
		//打印蚂蚁k的第i条路径
		cout << antColony[k].antPath[i];


		//将路径转化为字符串形式存入res中，方便打印输出
		res += to_string(antColony[k].antPath[i]);

		//若蚂蚁k的i+1条路径不为-1，代表访问过此点。即i--->i+1有路线。
		if (antColony[k].antPath[i + 1] != -1) {
			//打印"->"
			cout << "->";

			
			/*if (p->search(antColony[k].antPath[i + 1])!=0) {
				res += "->(可充电站点)";
			}*/

			// 若此点含有充电设施
			if (find(rechargeNumber.begin(), rechargeNumber.end(), antColony[k].antPath[i + 1]) != rechargeNumber.end()) {
				res += "->(可充电站点)";
			}
			else {
				res += "->";
			}

		}

	}
	cout << endl;
	cout << " 对应距离为：" << getAntLen(antColony[k]) << endl;

	//把每次距离结果保存到map中:关键字：路线，值：对应的路径长度
	routResult[res] = getAntLen(antColony[k]);
	res.swap(res);
	//return routResult;
	//delete p;
}
#pragma endregion

#pragma region 更新信息素矩阵
void updatePherMartix() {
	for (int i = 0; i < ANT_NUMBER; ++i) {
		//全局更新信息素矩阵
		if (antColony[i].flag == 1) {
			for (int j = 0; j < PATH_NUMBER - 1; ++j) {
				if (antColony[i].antPath[j] == -1 || antColony[i].antPath[j + 1] == -1)
					break;
				else
					nextPheromoneMatrix[antColony[i].antPath[j]][antColony[i].antPath[j + 1]]
					+= Q / getAntLen(antColony[i]);
			}
		}
	}
	//更新下一代信息素矩阵
	for (int i = 0; i < CITY_NUMBER; ++i) {
		for (int j = 0; j < CITY_NUMBER; ++j) {
			nextPheromoneMatrix[i][j] =
				(1 - ROU) * pheromoneMatrix[i][j] + nextPheromoneMatrix[i][j];
		}
	}

}
#pragma endregion

#pragma region 迭代过程
void evolution(int city) {


	cout << "开始进行迭代........." << endl;

	//初始化参数
	initAntsColony();
	initCity();
	initVariousMatrix();


	int gen = 0;
	while (gen < MAX_ITERATIONS_NUMBER) {
		int p = 0;
		while (p <15){
			for (int i = 0; i < ANT_NUMBER; ++i) {
				if(antColony[i].flag == 1)
					continue;
				nextCitySelect(i, city);
				initCity();
			}
			++p;
		}

		

		if (gen == MAX_ITERATIONS_NUMBER - 1) {
			cout << " 迭代完成，输出结果" << endl;
			printBestPath(getBestPathAntNumber(), city);

		}
		
		
		//更新信息素矩阵
		updatePherMartix();
		//蚁群初始化
		initAntsColony();
		pheromoneMatrix = nextPheromoneMatrix;
	    initMatrix(nextPheromoneMatrix, 0);
		++gen;
	}

}
#pragma endregion

#pragma region 释放vector内存空间，避免内存无限增加而崩溃
void deleArrary() {
	vector<vector<double>>().swap(cityDistance);
	vector<vector<double>>().swap(pheromoneMatrix);
	vector<vector<double>>().swap(nextPheromoneMatrix);
	vector<vector<double>>().swap(inspMartix);
	//map<string, int>().swap(routResult);
}
#pragma endregion

#pragma region 按从小到大的顺序将vec内部value值进行排序,并打印出结果
void sortMap(map<string, double>t) {
	//routeResult存入vec中，方便将结果进行排序
	vector<pair<string, double>>vec;
	for (const auto& n : t) {
		vec.push_back(make_pair(n.first, n.second));
	}
	//将结果按照路径，从小到达进行排序
	sort(vec.begin(), vec.end(), [](const pair<string, double>& x, const pair<string, double>& y)
		-> double {
			return x.second < y.second;
		});

	//输出排序好的所有结果
	cout << "可供选择的路径中，从小到大为：" << endl;
	int i = 1;
	for (auto v : vec) {
		cout << "第" << i << "条路径为：         ";
		cout << v.first << " " << "距离为："
			<< v.second << endl;
		++i;
	}


	cout << endl;
	cout << "建议采纳的最佳路线为：" << vec[0].first << " "
		<< "此路线距离最短，耗时最小为："
		<< vec[0].second << endl;


	for (auto v : vec)
	{
		if (vec[0].first.find("可充电站点") != -1) {
			break;
		}
		else if (v.first.find("可充电站点") != -1) {
			cout << "若需要中途停靠充电，建议采纳的最佳路线为："
				<< v.first
				<< "    此路线含有充电设施,总的路径为："
				<< v.second
				<< endl;
			break;
		}
	}

	t.swap(t);
	vector<pair<string, double>>().swap(vec);
}
#pragma endregion

#pragma region 程序入口

	int main() {
	cout << " 请输入你要去往的城市：" << endl;
	int city;
	cin >> city;
	
	//不断循环迭代，将每次循环的最优结果保存到routResult中
	while (1) {
		//随机化种子，用于生成随机数
		srand((unsigned)time(NULL));
		//开始进入迭代过程
		evolution(city);
		//每次迭代完成后，清空数组数据
		deleArrary();
		//判断键盘响应
		if (_kbhit()) {
			//如果读取到键盘按下'q'或'Q'键，则退出循环，结束整个过程
			if (tolower(_getch()) == 'q') {
				break;
			}
		}
	}


	//int x = 20;
	//while (x) {
	//	//随机化种子，用于生成随机数
	//	srand((unsigned)time(NULL));
	//	//开始进入迭代过程
	//	evolution(city);
	//	//每次迭代完成后，清空数组数据
	//	deleArrary();
	//	--x;
	//}

	//将所有保存的结果按照从小到大进行排序输出，并打印出最佳结果
	sortMap(routResult);
	return 0;
}
#pragma endregion
