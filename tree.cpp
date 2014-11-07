#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

const int N=100000;
const int L=3;
const int K=10;					//kmeans的K


unsigned char **feature=new unsigned char *[N];			//保存特征向量
//unsigned char **query=new unsigned char *[1000];
int *nodeson=new int [N];		//保存kmeans每次所在子节点。
int *layer= new int [L];		//保存树每层的开始点
//int *star=new int [2*N];
//int *Wi=new int [2*N];
int NN;							//节点序号
//int Query;
//int Imag;
struct TreeNode					//树节点
{
    float center[128];
    float summ[128];
    int son[K];
    int ran;
    int numm;
    int kk;
}tree[2*N];
struct Sor 						//特征属于那个节点与图片
{
	int node;
	int whichimage;
}sor[N];
/*struct Base
{
	float ba;
	int id;
}base[10000];*/
void init(int a,int b,unsigned char **T)
{
    for(int i=0;i<a;i++)
    {
        T[i]=new unsigned char[b];
    }
}
void final(int a,unsigned char **T)
{
	for(int i=0;i<a;i++)
	{
		delete[] T[i];
	}
	delete[] T;
}
void initial()
{
	srand((int)time(0));
	layer[0]=0;
	layer[1]=1;
	tree[0].numm=N;
	tree[0].kk=0;
	NN=0;
	for(int i=0;i<N;i++)
	{
		sor[i].node=0;
	}
	memset(nodeson,0,sizeof(nodeson));
}
/*bool cmp(Sor s1,Sor s2)
{
	return s1.node<s2.node;
}
bool cmp2(Base b1,Base b2)
{
	return b1.ba<b2.ba;
}*/
void file_intput()
{
	ifstream in("SampleDSift_100K", ios::in|ios::binary);
	for(int i=0;i<N;i++)			
	{
		in.read((char*)feature[i],sizeof(unsigned char)*128);
	}
	in.close();
}
int find_closest(int s,int xx,unsigned char **T)				//找最近的子节点
{
	int num;
	float minsum;
	for(int i=0;i<K;i++)
	{
		float sum=0;
		for(int j=0;j<128;j++)
		{
			float ss=((float)T[s][j]-tree[tree[xx].son[i]].center[j]);
			sum+=ss*ss;
		}
		if(i)
		{
			if(sum<minsum)
			{
				minsum=sum;
				num=i;
			}
		}
		else
		{
			minsum=sum;
			num=i;
		}
	}
	return tree[xx].son[num];
}
void find_init_central(int l)
{
	for(int i=layer[l];i<layer[l+1];i++)   
	{
		if(tree[i].numm>=K)
		{
			for(int j=0;j<K;j++)
			{
				tree[i].son[j]=++NN;				//分配子节点
				tree[tree[i].son[j]].kk=0;
				int ww;
				int aa=1;
				while(aa)							//random
				{
					aa=0;
					ww=rand()%tree[i].numm+1;
					for(int k=0;k<j;k++)
					{
						if(ww==tree[tree[i].son[k]].ran)
						{
							aa=1;
						}
					}
				}
				tree[tree[i].son[j]].ran=ww; 
			}
		}
	}
	layer[l+2]=NN+1;
	for(int i=0;i<N;i++)							//K个son的初始中心
	{
		if(tree[sor[i].node].numm>=K)
		{
			tree[sor[i].node].kk++;
			int ww=-1;
			for(int j=0;j<K;j++)
			{
				if(tree[tree[sor[i].node].son[j]].ran==tree[sor[i].node].kk)
				{
					ww=j;
					break;
				}
			}
			if(ww>=0)
			{
				for(int j=0;j<128;j++)
				{
					tree[tree[sor[i].node].son[ww]].center[j]=(float)feature[i][j];
				}
			}
		}
	}
}
void build_tree(int l)
{
	if(l==L)
	{
		return;
	}
	find_init_central(l);
	int ff=1;
	int stop=1;
	while(ff)
	{
		if(stop>50)
		{
			break;
		}
		stop++;
		ff=0;
		for(int i=layer[l+1];i<layer[l+2];i++)			//初始化numm，summm
		{
			tree[i].numm=0;
			for(int j=0;j<128;j++)
			{
				tree[i].summ[j]=0.0;
			}		
		}
		for(int i=0;i<N;i++)					//判断和那个son中心近
		{
			if(tree[sor[i].node].numm>=K)
			{

				int p=find_closest(i,sor[i].node,feature);
				tree[p].numm++;
				for(int j=0;j<128;j++)
				{
					tree[p].summ[j]+=(float)feature[i][j];
				}
				if(nodeson[i]!=p)
				{
					ff=1;
					nodeson[i]=p;
				}
			}
		}
		for(int i=layer[l+1];i<layer[l+2];i++)				//计算出中心
		{
			for(int j=0;j<128;j++)
			{
				tree[i].center[j]=tree[i].summ[j]/tree[i].numm;
			}
		}
		float just_test=0.0;			//test
		for(int i=0;i<N;i++)
		{
			float s12=0.0;
			for(int j=0;j<128;j++)
			{
				float sss=feature[i][j]-tree[nodeson[i]].center[j];
				s12+=sss*sss;
			}
			just_test+=sqrt(s12);
		}
		cout<<just_test<<endl;
	}
	for(int i=0;i<N;i++)			//到son节点
	{
		sor[i].node=nodeson[i];
	}
	build_tree(l+1);
}
/*void next_step()
{
	sort(sor,sor+N,cmp);
	star[sor[0].node]=0;
	Wi[sor[0].node]++;
	for(int i=0;i<N-1;i++)
	{
		if(sor[i+1].node!=sor[i].node)
		{
			star[sor[i+1].node]=i+1;

		}
		if(sor[i+1].whichimage!=sor[i].whichimage)
		{
			Wi[sor[i+1].node]++;
		}
	}
}
int find_left(int s,int now,int lay)
{
	if(lay==L||tree[now].numm<K)
	{
		return now;
	}
	int no=find_closest(s,now,query);
	return find_left(s,no,lay+1);
}
void call()
{
	for(int i=0;i<Imag;i++)
	{
		base[i].id=i;
	}
	for(int i=0;i<Query;i++)
	{
		int kkk=findleft(i,0,0);
		for(int j=star[kkk];j<star[kkk+1]&&j<N;j++)
		{
			float rr=log(Imag/Wi[kkk]);
			base[sor[j].whichimage].ba+=rr*rr;
		}
	}
	for(int i=0;i<Imag;i++)
	{
		base[i].ba=2-2*base[i].ba;
	}
	sort(base,base+Imag,cmp2);
}*/
int main()
{
	init(N,128,feature);
	file_intput();
	//init(1000,128,query);
	/*scanf("%d",&Imag);
	for(int i=0;i<N;i++)
	{
		scanf("%d",&sor[i].whichimage);
	}*/
	initial();
	build_tree(0);
	cout<<NN<<endl;					//test
	//nex_tstep();
	/*scanf("%d",&Query);
	for(int i=0;i<Query;i++)
	{
		for(int j=0;j<128;j++)
		{
			scanf("%d",&query[i][128]);
		}
	}*/
	//call();
	/*for(int i=0;i<Imag;i++)
	{
		printf("%d ",base[i].id);
	}
	printf("\n");*/
	final(N,feature);
	//final(1000,query);
	return 0;
}
