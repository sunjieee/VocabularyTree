#include <bits/stdc++.h>
#include <dirent.h>

using namespace std;

const int INF = 2000000000;

struct sift
{
	unsigned char feature[128];
	float x, y, scale, angle;
};
struct node
{
	unsigned char cluster_centre[128];
	vector<int> image_id;
	vector<int> image_frequcy;
	node **child;
};
ifstream in;
ofstream out;
//int sss = 0;

void find_leaf_node(node *q, sift s, int id)
{
	if (q->child == NULL)
	{
		int size = q->image_id.size();
		if (size == 0 || q->image_id[size - 1] != id)
		{
			q->image_id.push_back(id);
			q->image_frequcy.push_back(1);
		}
		else
		{
			q->image_frequcy[size - 1]++;
		}
		return;
	}
	int mi =INF, tt;
	for (int i = 0; i < 10; i++)
	{
		int sum = 0;
		for (int j = 0; j < 128; j++)
		{
			int mid = q->child[i]->cluster_centre[j];
			sum += (s.feature[j] - mid) * (s.feature[j] - mid);
		}
		if (mi > sum)
		{
			mi = sum;
			tt = i;
		}
	}
	find_leaf_node(q->child[tt], s, id);
}

void read_file_list(char *basePath, node *q, int &id, vector<string> &image_name)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];
    if ((dir = opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }
    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
        {
            continue;
        }
        else if (ptr->d_type == 8)    ///file
        {
        	for (int i = 0; ptr->d_name[i] != '\0'; i++)          // my code(all 'for')
        	{
        		if (strcmp(ptr->d_name + i, ".sift") == 0)
        		{
        			memset(base,'\0',sizeof(base));
        			strcpy(base,basePath);
        			strcat(base,"/");
            		strcat(base,ptr->d_name);
        			ifstream in_sift(base, ios::in | ios::binary);        
        			int num;
        			in_sift.read((char*)&num, sizeof(int));
        			sift sift_db;
        			for (int j = 0; j < num; j++)
        			{
        				in_sift.read((char*)&sift_db, sizeof(sift));
        				find_leaf_node(q, sift_db, id);
        			}
        			image_name.push_back(ptr->d_name);
        			//printf("id%d \n", id);
        			id++;
        			in_sift.close();
        			break;
        		}
        	}													
            //printf("d_name:%s/%s\n",basePath,ptr->d_name);
        }
        else if (ptr->d_type == 10)    ///link file
        {
            //printf("d_name:%s/%s\n",basePath,ptr->d_name);
        }
        else if (ptr->d_type == 4)    ///dir
        {
            memset(base,'\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            read_file_list(base, q, id, image_name);
        }
    }
    closedir(dir);
}

void sift_file_read(node *q, int &id, vector<string> &image_name)
{
	DIR *dir;
    char basePath[1000];
    //get the file list
    memset(basePath,'\0', sizeof(basePath));
    strcpy(basePath, "/home/sunjie/ukbench_all");
    read_file_list(basePath, q, id, image_name);
}

void initial_node(node *q)
{
	q->child = NULL;
	q->image_id.clear();
	q->image_frequcy.clear();
}

void load_codebook_file(node *q)
{
	q->child = new node *[10];
	for (int i = 0; i < 10; i++)
	{
		q->child[i] = new node;
		initial_node(q->child[i]);
		in.read((char*)q->child[i]->cluster_centre, sizeof(unsigned char) * 128);
		bool is_leaf;
		in.read((char*)&is_leaf, sizeof(bool));
		//sss++;
		if (is_leaf == 0)
		{
			load_codebook_file(q->child[i]);
		}
	}
}

void make_norm(node *q, float *norm, int id)
{
	if (q->child == NULL)
	{
		int size = q->image_id.size();
		for (int i = 0; i < size; i++)
		{
			int k = q->image_id[i];
			float temp = q->image_frequcy[i] * log(1.0 * id / size);
			norm[k] += temp * temp;
		}
		return;
	}
	for (int i = 0; i < 10; i++)
	{
		make_norm(q->child[i], norm, id);
	}
}

void save_index_tree(node *q, float *norm, int id)
{
	for (int i = 0; i < 10; i++)
	{
		out.write((char*)q->child[i]->cluster_centre, sizeof(unsigned char) * 128);
		int num = q->child[i]->image_id.size();
		out.write((char*)&num, sizeof(int));
		int *temp = new int [num];
		for (int j = 0; j < num; j++)
		{
			temp[j] = q->child[i]->image_id[j];
		}
		out.write((char*)temp, sizeof(int) * num);
		float *norm_db = new float [num];
		for (int j = 0; j < num; j++)
		{
			int k = q->child[i]->image_id[j];
			norm_db[j] = q->child[i]->image_frequcy[j] * log(1.0 * id / num) / sqrt(norm[k]);
		}
		out.write((char*)norm_db, sizeof(float) * num);
		bool is_leaf;
		if (q->child[i]->child == NULL)
		{
			is_leaf = 1;
		}
		else
		{
			is_leaf = 0;
		}
		out.write((char*)&is_leaf, sizeof(bool));
		//sss++;
		if (is_leaf == 0)
		{
			save_index_tree(q->child[i], norm, id);
		}
		delete[] temp;
		delete[] norm_db;
	}
}

void save_image_name(int id, vector<string> image_name)
{
	for (int i = 0; i < id; i++)
	{
		int size = image_name[i].size();
		char *temp = new char [size];
		for (int j = 0; j < size; j++)
		{
			temp[j] = image_name[i][j];
		}
		out.write((char*)&size, sizeof(int));
		out.write((char*)temp, sizeof(char) * size);
		delete[] temp;
	}
}

int main()
{
	in.open("output.data", ios::in | ios::binary);
	out.open("output_index.data", ios::out | ios::binary);
	vector<string> image_name;
	image_name.clear();
	node *root = new node;
	initial_node(root);
	load_codebook_file(root);
	int id = 0;
	sift_file_read(root, id, image_name);
	out.write((char*)&id, sizeof(int));
	float *norm = new float [id];
	for (int i = 0; i < id; i++)
	{
		norm[i] = 0.0; 
	}
	make_norm(root, norm, id);
	save_index_tree(root, norm, id);
	save_image_name(id, image_name);
	//printf("sss%d",sss);
	delete[] norm;
	in.close();	
	out.close();
	return 0;
}