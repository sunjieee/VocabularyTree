#include <bits/stdc++.h>

using namespace std;

const int MAX_N = 5000000;
const int INF = 2000000000;

struct node
{
	unsigned char cluster_centre[128];
	bool is_leaf;
};
ofstream out;

void load_feature_file(unsigned char **feature)
{
	for (int i = 0; i < MAX_N; i++)
    {
        feature[i] = new unsigned char [128];
    }
	ifstream in("DSiftSampe_5M_frq_10_ImgNet", ios::in | ios::binary);
	for (int i = 0; i < MAX_N; i++)
	{
		in.read((char*)feature[i], sizeof(unsigned char) * 128);
	}
	in.close();
}

void output(node *codebook, int id)
{
	out.write((char*)&codebook[id], sizeof(node));
}

void creat_initial_centre(unsigned char **feature, int *index, int feature_num, node *codebook)
{
	int *temp = new int [10];
	int num = 0;
	while (num < 10)
	{
		int tt = rand() % feature_num;
		int flag = 0;
		for (int i = 0; i < num; i++)
		{
			if (tt == temp[i])
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0)
		{
			temp[num++] = tt;
		}
	}
	for (int i = 0; i < 10; i++)
	{
		int k = index[temp[i]];
		for (int j = 0; j < 128; j++)
		{
			codebook[i].cluster_centre[j] = feature[k][j];
		}
	}
	delete[] temp;
}

void k_means(unsigned char **feature, int *index, int feature_num, int **index_ch, int *feature_num_ch, node *codebook)
{
	creat_initial_centre(feature, index, feature_num, codebook);
	int cas = 30;
	int *num_cycle = new int [10];
	int **sum_cycle = new int *[10];
	int *index_temp = new int [feature_num];
	int *num_temp = new int [10];
	for (int i = 0; i < 10; i++)
	{
		sum_cycle[i] = new int [128];
	}
	while (cas--)
	{
		double pppp = 0.0;
		int flag = 0;
		for (int i = 0; i < 10; i++)
		{
			num_cycle[i] = 0;
			for (int j = 0; j < 128; j++)
			{
				sum_cycle[i][j] = 0;
			}
		}
		for (int i = 0; i < feature_num; i++)
		{
			int k = index[i], mi = INF, id;
			for (int j = 0; j < 10; j++)
			{
				int sum = 0;
				for (int l = 0; l < 128; l++)
				{
					int mid = codebook[j].cluster_centre[l];
					sum += (feature[k][l] - mid) * (feature[k][l] - mid);
				}
				if (mi > sum)
				{
					mi = sum;
					id = j;
				}
			}
			num_cycle[id]++;
			pppp += sqrt((double)mi);
			for (int j = 0; j < 128; j++)
			{
				sum_cycle[id][j] += feature[k][j];
			}
			if (index_temp[i] != id)
			{
				flag = 1;
				index_temp[i] = id;
			}
		}
		printf("cas%d %lf\n", cas, pppp / feature_num);
		for (int i = 0; i < 10; i++)     //test
		{
			printf("i%d:%d*\n", i, num_cycle[i]);
		}
		if (flag == 0)
		{
			break;
		}
		for (int i = 0; i < 10; i++)
		{
			if (num_cycle[i])
			{
				for (int j = 0; j < 128; j++)
				{
					codebook[i].cluster_centre[j] = sum_cycle[i][j] / num_cycle[i];
				}
			}
			else
			{
				int k = index[rand() % feature_num];
				for (int j = 0; j < 128; j++)
				{
					codebook[i].cluster_centre[j] = feature[k][j];
				}

			}
		}
	}
	for (int i = 0; i < 10; i++)
	{
		feature_num_ch[i] = 0;
		num_temp[i] = 0;
	}
	for (int i = 0; i < feature_num; i++)
	{
		feature_num_ch[index_temp[i]]++;
	}
	for (int i = 0; i < 10; i++)
	{
		index_ch[i] = new int [feature_num_ch[i]];
	}
	for (int i = 0; i < feature_num; i++)
	{
		int tem = index_temp[i];
		index_ch[tem][num_temp[tem]++] = index[i];
	}
	for (int i = 0; i < 10; i++)
	{
		delete[] sum_cycle[i];
	}
	delete[] sum_cycle;
	delete[] index_temp;
	delete[] num_cycle;
	delete[] num_temp;
}

void train_codebook(unsigned char **feature, int *index, int feature_num, int layor, int layor_max)
{
	int **index_ch = new int *[10];
	int *feature_num_ch = new int [10];
	node *codebook = new node [10];
	k_means(feature, index, feature_num, index_ch, feature_num_ch, codebook);
	for (int i = 0; i < 10; i++)
	{
		if (layor < layor_max && feature_num_ch[i] >= 10)
		{
			codebook[i].is_leaf = 0;
		}
		else
		{
			codebook[i].is_leaf = 1;
		}
		output(codebook, i);
		if (layor < layor_max && feature_num_ch[i] >= 10)
		{
			train_codebook(feature, index_ch[i], feature_num_ch[i], layor + 1, layor_max);
		}
	}
	for (int i = 0; i < 10; i++)
	{
		delete[] index_ch[i];
	}
	delete[] index_ch;
	delete[] feature_num_ch;
	delete[] codebook;
}

int main()
{
	unsigned char **feature = new unsigned char *[MAX_N];
	srand((int)time(0));
	int layor_max = 5;
	load_feature_file(feature);
	int *index = new int [MAX_N];
	for (int i = 0; i < MAX_N; i++)
	{
		index[i] = i;
	}
	out.open("output.data", ios::out | ios::binary);
	train_codebook(feature, index, MAX_N, 1, layor_max);
	for (int i = 0; i < MAX_N; i++)
	{
		delete[] feature[i];
	}
	delete[] feature;
	delete[] index;
	out.close();
	return 0;
}