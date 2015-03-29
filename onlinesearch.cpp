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
	int image_num;
	int *image_id;
	float *norm_db;
	int query_num;
	node **child;
};
struct result
{
	float score;
	int id;
};
ifstream in;
int sssss;
double mmap; //

void initial_node(node *q)
{
	q->child = NULL;
	q->image_id = NULL;
	q->norm_db = NULL;
	q->query_num = 0;
}

bool cmp(result r1, result r2)
{
	return r1.score < r2.score;
}

void load_index_file(node *q)
{
	q->child = new node *[10];
	for (int i = 0; i < 10; i++)
	{
		q->child[i] = new node;
		initial_node(q->child[i]);
		in.read((char*)q->child[i]->cluster_centre, sizeof(unsigned char) * 128);
		int num;
		in.read((char*)&num, sizeof(int));
		q->child[i]->image_num = num;
		if (num)
		{
			q->child[i]->image_id = new int [num];
			q->child[i]->norm_db = new float [num];
			in.read((char*)q->child[i]->image_id, sizeof(int) * num);
			in.read((char*)q->child[i]->norm_db, sizeof(float) * num);
		}
		bool is_leaf;
		in.read((char*)&is_leaf, sizeof(bool));
		//sssss++;
		if (is_leaf == 0)
		{
			load_index_file(q->child[i]);
		}
	}
}

void clear_iamge_information(node *q)
{
	if (q->child == NULL)
	{
		q->query_num = 0;
		return;
	}
	for (int i = 0; i < 10; i++)
	{
		clear_iamge_information(q->child[i]);
	}
}

void find_leaf_node(node *q, sift s, int id)
{
	if (q->child == NULL)
	{
		q->query_num++;
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

void make_norm(node *q, float &norm_q, int id)
{
	if (q->child == NULL)
	{
		if (q->image_num)
		{
			float temp = q->query_num * log(1.0 * id / q->image_num);
			norm_q += temp * temp;
		}
		return;
	}
	for (int i = 0; i < 10; i++)
	{
		make_norm(q->child[i], norm_q, id);
	}
}

/*void load_search_image(node *q, int id)
{
	ifstream in_search("ukbench00040.jpg.sift", ios::in | ios:: binary);
	int num;
	in_search.read((char*)&num, sizeof(int));
	sift sift_search;
	for (int i = 0; i < num; i++)
	{
		in_search.read((char*)&sift_search, sizeof(sift));
		find_leaf_node(q, sift_search, id);
	}
}*/

void load_image_name(int id, char **image_name)
{
	for (int i = 0; i < id; i++)
	{
		int size;
		in.read((char*)&size, sizeof(int));
		image_name[i] = new char [size];
		in.read((char*)image_name[i], sizeof(char) * size);
	}
}

void dfs_tree(node *q, float norm_q, result *res, int id)
{
	if (q->child == NULL)
	{
		int num = q->image_num;
		for (int i = 0; i < num; i++)
		{
			int k = q->image_id[i];
			//printf("i%d:%d:%f:%f:%f\n", i, q->query_num, log(1.0 * id / num), sqrt(norm_q), q->norm_db[i]);
			res[k].score -= 2.0 * q->query_num * log(1.0 * id / num) / sqrt(norm_q) * q->norm_db[i];
		}
		return;
	}
	for (int i = 0; i < 10; i++)
	{
		dfs_tree(q->child[i], norm_q, res, id);
	}
}

void creat_score(node *q, float norm_q, int id, char **image_name, char *d_name)
{
	result *res = new result [id];
	for (int i = 0; i < id; i++)
	{
		res[i].score = 2.0;
		res[i].id = i;
	}
	dfs_tree(q, norm_q, res, id);
	sort(res, res + id, cmp);
	int sum = 0;
	for (int i = 7; i < 12; i++)
	{
		sum *= 10;
		sum += d_name[i] - '0';
	}
	double pppp = 0.0;
	int sss = 0;
	for (int i = 0; i < id; i++)
	{
		int sum1 = 0;
		for (int j = 7; j < 12; j++)
		{
			sum1 *= 10;
			sum1 += image_name[res[i].id][j] - '0';
		}
		if (sum / 4 == sum1 / 4)
		{
			//printf("%d\n", sum1);
			sss++;
			pppp += 1.0 * sss / (i + 1); 
		}
		if (i < 4)
		{
			//printf("%s:  ", image_name[res[i].id]);
			//printf("%f\n", res[i].score);
		}
		
	}
	printf("%d\n", sss);
	pppp /= sss;
	printf("pppp%lf\n", pppp);
	mmap += pppp;
	delete[] res;
}

void read_file_list(char *basePath, node *q, int id, char **image_name)
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
        			clear_iamge_information(q);
        			sift sift_q;
        			for (int j = 0; j < num; j++)
        			{
        				in_sift.read((char*)&sift_q, sizeof(sift));
        				find_leaf_node(q, sift_q, id);
        			}
    				printf("\n%s\n", ptr->d_name);
    				//printf("num%d\n", num);
        			float norm_q = 0;
        			make_norm(q, norm_q, id);
					creat_score(q, norm_q, id, image_name, ptr->d_name);   //
        			
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

void load_search_image(node *q, int id, char **image_name)
{
	DIR *dir;
    char basePath[1000];
    //get the file list
    memset(basePath,'\0', sizeof(basePath));
    strcpy(basePath, "/home/sunjie/ukbench_all");
    read_file_list(basePath, q, id, image_name);
}

int main()
{
	mmap = 0.0; //
	sssss = 0;
	//freopen("output.txt", "w", stdout);                 //
	in.open("output_index.data", ios::in | ios::binary);
	node *root = new node;
	int id;
	in.read((char*)&id, sizeof(int));
	char **image_name = new char *[id];
	//float norm_q = 0;
	initial_node(root);
	load_index_file(root);
	load_image_name(id, image_name);
	//load_search_image(root, id);
	load_search_image(root, id, image_name);     //
	//make_norm(root, norm_q, id);
	//creat_score(root, norm_q, id, image_name);
	for (int i = 0; i < id; i++)
	{
		delete[] image_name[i];
	}
	delete[] image_name;
	//printf("sssss%d", sssss);
	printf("map%lf\n", mmap / 10200);     //               
	//fclose(stdout);                          //
	in.close();
	return 0;
}