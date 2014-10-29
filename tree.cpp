#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <cmath>
using namespace std;

const int N=100000;
const int L=100;

unsigned char **feature=new unsigned char *[N];			//保存特征向量
//unsigned char **query=new unsigned char *[1000];
int *nodeson=new int [N];		//保存kmeans每次所在子节点。
int *layer= new int [L];		//保存树每层的开始点
int *star=new int [2*N];
int *Wi=new int [2*N];
int NN;							//节点序号
int k;							//kmeans的k
int Query;
int Imag;
struct TreeNode					//树节点
{
    unsigned char center[128];
    int summ[128];
    int son[100];
    int numm;
    int kk;
}tree[2*N];
struct sor 						//特征属于那个节点与图片
{
	int node;
	int whichimage;
}sor[N];
struct Base
{
	float ba;
	int id;
}base[10000];
void init(int a,int b,unsigned char **T)
{
    for(int i=0;i<a;i++)
    {
        T[i]=new unsigned char[b]();
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
bool cmp(Sor s1,Sor s2)
{
	return s1.node<s2.node;
}
bool cmp2(Base b1,Base b2)
{
	return b1.ba<b2.ba;
}
int findclosest(int s,int xx,unsigned char **T)				//找最近的子节点
{
	int num;
	int minsum;
	for(int i=0;i<k;i++)
	{
		int sum=0;
		for(int j=0;j<128;j++)
		{
			int ss=(T[s][j]-tree[tree[xx].son[i]].center[j]);
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
	return tree[sor[xx].node].son[num];
}
void buildtree(int l)
{
	if(l==L)
	{
		return;
	}
	for(int i=layer[l];i<layer[l+1];i++)   //分配子节点
	{
		if(tree[i].numm>=k)
		{
			for(int j=0;j<k;j++)
			{
				tree[i].son[j]=++NN;
				tree[tree[i].son[j]].kk=0;
			}
		}
	}
	layer[l+2]=NN+1;
	for(int i=0;i<N;i++)	//k个son的初始中心
	{
		if(tree[sor[i].node].numm>=k&&tree[sor[i].node].kk<k)
		{
			tree[sor[i].node].kk++;
			for(int j=0;j<128;j++)
			{
				tree[tree[sor[i].node].son[tree[sor[i].node].kk]].center[j]=feature[i][j];
			}
		}
	}
	int ff=1;
	while(ff)
	{
		ff=0;
		for(int i=layer[l+1];i<layer[l+2];i++)			//初始化numm，summm
		{
			tree[i].numm=0;
			for(int j=0;j<128;j++)
			{
				tree[i].summ[j]=0;
			}		
		}
		for(int i=0;i<N;i++)					//判断和那个son中心近
		{
			if(tree[sor[i].node].numm>=k)
			{

				int p=findclosest(i,sor[i].node,feature);
				tree[p].numm++;
				for(int j=0;j<128;j++)
				{
					tree[p].summ[j]+=feature[i][j];
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
	}
	for(int i=0;i<N;i++)			//到son节点
	{
		sor[i].node=nodeson[i];
	}
	buildtree(l+1);
}
void nextstep()
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
int findleft(int s,int now,int lay)
{
	if(lay==L||tree[now].numm<k)
	{
		return now;
	}
	int no=findclosest(s,now,query);
	return findleft(s,no,lay+1);
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
}
int main()
{
	/*init(N,128,feature);
	init(1000,128,query);
	scanf("%d",&Imag);
	for(int i=0;i<N;i++)
	{
		scanf("%d",&sor[i].whichimage);
	}
	for(int i=0;i<N;i++)
	{
		for(int j=0;j<128;j++)
		{
			scanf("%f",&feature[i][j]);
		}
	}*/
	layer[0]=0;
	layer[1]=1;
	tree[0].numm=N;
	NN=0;
	for(int i=0;i<N;i++)
	{
		sor[i].node=0;
	}
	memset(nodeson,0,sizeof(nodeson));
	buildtree(0);
	//nextstep();
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
	//final(N,feature);
	//final(1000,query);
	return 0;
}
