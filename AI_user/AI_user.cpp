#include "stdafx.h"
#include "Api.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <bitset>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
extern "C" AI_USER_API PlayerName NameAI() {
	PlayerName temp;
	strcpy_s(temp.TeamName, "dragoncat");		//队名
	strcpy_s(temp.my1Name,  "dragon");			//1号AI的名字
	strcpy_s(temp.my2Name,  "cat");			//2号AI的名字
	return temp;
}
const int fx[5][2] = {{0,0},{-1,0},{1,0},{0,-1},{0,1}}; //行走
const int inf = 0x13131313;	// 极大值
const int hashh[2] = {0,10};
const int wh[2]={0,2};
const int stoptimetestbskill=6;
const int timebskill1start=8600;
const int ekillarea=5;
const int timefoodoff=20;
int killmap[boderY][boderX];// 运行魔法地图
int elen[2]; //敌人魔法长度	
bool checkeskillans;
int mapspace[boderY][boderX];
bool can[2]={false,false};

/*
关于m的使用
0 1 2 寻址时判重复

40 41 记录A组bigskillcd

50-59 检测敌人开大使用 
60-79 同上

100-101种树点的坐标
110-----检测吸人大招、、
*/


int checkemagiclen(User *u)//检测敌人魔法长度
{
	elen[0]=8; elen[1]=8;
	for (int i=0;i<2;i++){
		if (u->egoods_num[i]>0)
			for (int j=0;j<u->egoods_num[i];j++)
				if (u->egoods_tpye[i][j]==BLUE) elen[i]+=2;
		if (elen[i]>30) elen[i]=30;
	}
	return 0;
}
int imagine(User *u,int ewho,int ey,int ex,int my,int mx,int len)
{
	if (len<=0) return 0;
	int kill[boderY*boderX][2];
	int maptmp[boderY][boderX];// 地图点标记
	int deep=1; //队列长度 
	int pd=inf;//标志
	fill(maptmp[0], maptmp[boderY], inf); 
	kill[0][0]=ey; kill[0][1]=ex;
	maptmp[ey][ex]=0;
	for (int i=0;i<deep;i++){
		for (int j=1;j<5;j++){
			int tmpy=kill[i][0] +fx[j][0];
			int tmpx=kill[i][1] +fx[j][1];
			if (!(tmpy==0 && tmpx==0) && tmpy >= 0 && tmpy < boderY && tmpx >= 0 && tmpx < boderX
				&& u->mapInfo[tmpy][tmpx] != TREE && maptmp[tmpy][tmpx] == inf && killmap[tmpy][tmpx]!=0){
				maptmp[tmpy][tmpx]=maptmp[kill[i][0]][kill[i][1]]+1;
				if (maptmp[tmpy][tmpx]>len || maptmp[tmpy][tmpx]>ekillarea) {  pd=0; break; }
				kill[deep][0]=tmpy;
				kill[deep][1]=tmpx;
				deep++;
				killmap[tmpy][tmpx]=1; //// ===1 区别于已经有魔法的0
				if (tmpy==my && tmpx==mx){
					killmap[tmpy][tmpx]=0;
					pd=0; 
					break;
				}
			}
		}
		if (pd==0) break;
	}
	return 0;
}
int findkillme(User *u,int ewho,int y, int x) //敌人攻击预测以及己方攻击未写。
{
	if (u->emagic_num[ewho]!=0){
			int yy=u->emagic_y[ewho][u->emagic_num[ewho]-1];
			int xx=u->emagic_x[ewho][u->emagic_num[ewho]-1];
			for (int j=0;j<u->emagic_num[ewho];j++) killmap[u->emagic_y[ewho][j]][u->emagic_x[ewho][j]]=0;
			imagine(u,ewho,yy,xx,y,x,elen[ewho]-u->emagic_num[ewho]);
		}
	return 0;
}
Order curvekill(User *u)
{
	Order tmp; tmp.act=STOP;
	if (u->y[u->who]==u->y[!u->who] && u->x[u->who]==u->x[!u->who]) return tmp;
	int maptmp[boderY][boderX];// 地图点标记
	if (u->power[u->who]>=10 && u->magic_num[u->who]==0){
		int kill[boderY*boderX][4];
		int deep=1; //队列长度 
		int pd=inf;  //标志
		fill(maptmp[0], maptmp[boderY], inf);  //记录宽度
		kill[0][0]=u->y[u->who]; kill[0][1]=u->x[u->who]; kill[0][2]=0; kill[0][3]=0;
		maptmp[u->y[u->who]][u->x[u->who]]=0;
		for (int i=0;i<=deep;i++){
			//for (int j=1;j<5;j++){
			for (int jj=4;jj>=0;jj--){
				int j=jj;
				//if (i%2==0) j=jj; else j=5-jj;
				int tmpy=kill[i][0] +fx[j][0];
				int tmpx=kill[i][1] +fx[j][1];
				if (!(tmpy==0 && tmpx==0) && tmpy >= 0 && tmpy < boderY && tmpx >= 0 && tmpx < boderX && u->mapInfo[tmpy][tmpx] != 2 && maptmp[tmpy][tmpx] == inf && !(tmpy==u->y[!u->who] && tmpx==u->x[!u->who])){
					maptmp[tmpy][tmpx]=maptmp[kill[i][0]][kill[i][1]]+1;
					if (u->magic_len[u->who]<=maptmp[tmpy][tmpx]) { pd=0; break;}  //此处小于等于还是小于有待研究
					kill[deep][0]=tmpy;
					kill[deep][1]=tmpx;
					kill[deep][2]=i;
					kill[deep++][3]=j;
					if ((tmpy==u->ey[0] && tmpx==u->ex[0])||(tmpy==u->ey[1] && tmpx==u->ex[1])){
						tmp.act=ATC; 
						deep--;
						int magicmap[boderY][boderX];
						fill(magicmap[0],magicmap[boderY],inf);
						for (int k=maptmp[tmpy][tmpx];k>=0;k--){            //此处魔法长度不一定达到最大长度，剩余魔法长度试用随机化
							magicmap[kill[deep][0]][kill[deep][1]]=0;
							//killmap[kill[deep][0]][kill[deep][1]]=0;   //修理中、、、、、
							tmp.dir[k]=kill[deep][3]; 
							deep=kill[deep][2];
						}
					///////////////////////////////////////////剩余魔法长度处理、、、(非随机化)
					 
						int k=maptmp[tmpy][tmpx]+1;  int kkk=inf; int kkkk;
						while(k<u->magic_len[u->who]){
							kkkk=inf;
							for (int kk=4;kk>=0;kk--)
							if (tmpy+fx[kk][0]>=0 && tmpy+fx[kk][0]<boderY && tmpx+fx[kk][1]>=0 && tmpx+fx[kk][1]<boderX && u->mapInfo[tmpy+fx[kk][0]][tmpx+fx[kk][1]]!=2 && magicmap[tmpy+fx[kk][0]][tmpx+fx[kk][1]]==inf){
								tmpy=tmpy+fx[kk][0];
								tmpx=tmpx+fx[kk][1];
								magicmap[tmpy][tmpx]=0;
								tmp.dir[k++]=kk;
								if (k>u->magic_len[u->who]) { kkk=0; break; }
								kkkk=0;
								break;
							}
							if (kkkk=inf) break;
							if (kkk==0) break;
						}
					
					
					///////////////////////////////////////////
						return tmp;
					}
				
				}
			}
			if (pd==0) break;
		}
		}
	return tmp;
}
Order goto_des(int x, int y, User * u) {
	if (x == u->x[u->who] && y == u->y[u->who]) {
		Order o; o.act = STOP;
		return o;
	}
	int maptmp[boderY][boderX];
	int que[boderY * boderX][2], qn=0;
	Order ord[5];
	ord[0].act = STOP; ord[1].act = UP;
	ord[2].act = DOWN; ord[3].act = LEFT;
	ord[4].act = RIGHT;
	fill(maptmp[0], maptmp[boderY], inf);
	maptmp[y][x] = 0;
	que[qn][0] = y;
	que[qn++][1] = x;
	for ( int i=0; i<qn; i++) {
		for ( int j=1; j<=4; j++) {
			int tmpy = que[i][0] + fx[j][0];
			int tmpx = que[i][1] + fx[j][1];
			if (tmpy >= 0 && tmpy < boderY && tmpx >= 0 && tmpx < boderX && 
				u->mapInfo[tmpy][tmpx] == LAND && maptmp[tmpy][tmpx] == inf && killmap[tmpy][tmpx]== inf) {
				maptmp[tmpy][tmpx] = maptmp[que[i][0]][que[i][1]] + 1;
				que[qn][0] = tmpy;
				que[qn++][1] = tmpx;
				if (maptmp[u->y[u->who]][u->x[u->who]] != inf) break;
				
			}
		}
		if (maptmp[u->y[u->who]][u->x[u->who]] != inf) break;
	}
	for ( int i=1; i<=4; i++) {
		int tmpy = u->y[u->who] + fx[i][0];
		int tmpx = u->x[u->who] + fx[i][1];
		if (tmpx >= 0 && tmpx < boderX && tmpy >= 0 && tmpy < boderY && maptmp[tmpy][tmpx] + 1 == maptmp[u->y[u->who]][u->x[u->who]]) return ord[i];
	}
	return ord[0];
}
Order goto_des2(User *u,int y,int x)
{
	Order tmp;  tmp.act=STOP;
	int ecan;
	if (can[0]) ecan=0; else ecan=1;
	string dir1,dir2; 
	if (u->emagic_y[ecan][u->emagic_num[ecan]-1]>y) dir1="down"; else dir1="up";
	if (u->emagic_x[ecan][u->emagic_num[ecan]-1]>x) dir2="right"; else dir2="left";
	int kdir;
	int ey=u->emagic_y[ecan][u->emagic_num[ecan]-2];
	int ex=u->emagic_x[ecan][u->emagic_num[ecan]-2];
	int eyy=u->emagic_y[ecan][u->emagic_num[ecan]-1];
	int exx=u->emagic_x[ecan][u->emagic_num[ecan]-1];
	for (int i=1; i<=4;i++) if (ey+fx[i][0]==eyy && ex+fx[i][1]==exx) {kdir=i; break;}
	
	if (dir1=="down" && dir2=="right"){
		if (kdir==LEFT){
			int tmpy=y-1; int tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=UP; return tmp;
			}
			tmpy=y; tmpx=x-1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=LEFT; return tmp;
			}
		}else
		{
			int tmpy=y; int tmpx=x-1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=LEFT; return tmp;
			}
			tmpy=y-1; tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=UP; return tmp;
			}
		}
	}
	if (dir1=="down" && dir2=="left"){
		if (kdir==RIGHT){
			int tmpy=y-1; int tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=UP; return tmp;
			}
			tmpy=y; tmpx=x+1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=RIGHT; return tmp;
			}
		}else
		{
			int tmpy=y; int tmpx=x+1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=RIGHT; return tmp;
			}
			tmpy=y-1; tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=UP; return tmp;
			}
		}
	}
	if (dir1=="up" && dir2=="right"){
		if (kdir==LEFT){
			int tmpy=y+1; int tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=DOWN; return tmp;
			}
			tmpy=y; tmpx=x-1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=LEFT; return tmp;
			}
		}else
		{
			int tmpy=y; int tmpx=x-1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=LEFT; return tmp;
			}
			tmpy=y+1; tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=DOWN; return tmp;
			}
		}
	}
	if (dir1=="up" && dir2=="left"){
		if (kdir==RIGHT){
			int tmpy=y+1; int tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=DOWN; return tmp;
			}
			tmpy=y; tmpx=x+1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=RIGHT; return tmp;
			}
		}else
		{
			int tmpy=y;int  tmpx=x+1;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=RIGHT; return tmp;
			}
		    tmpy=y+1; tmpx=x;
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]!=0){
				tmp.act=DOWN; return tmp;
			}
		}

	}
	for (int i=1;i<5;i++){
		int tmpy=y+fx[i][0];
		int tmpx=x+fx[i][1];
		if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && killmap[tmpy][tmpx]==inf){
			tmp.act=i; return tmp;
		}
	}
	return tmp;
}
Order movefoods(User *u,int goodstype)
{
	Order tmp; tmp.act=STOP;
	int num=1;  //宽搜指针
	int bfs[boderX*boderY][2];//队列
	int maptmp[boderY][boderX];// 地图点标记
	fill(maptmp[0], maptmp[boderY], inf); //数组清零
	int x=inf; int y=inf;
	int xx=0; int yy=0; int first=0;
	/////////////////////////////////////////////物品覆盖地图
	int mapfoods[boderY][boderX][2];
	for (int i=0;i<boderY;i++)
		for (int j=0;j<boderX;j++) mapfoods[i][j][0]=inf;
	for (int i=0;i<u->map_goods;i++){
		if (goodstype!=inf && u->map_goods_type[i]!=goodstype) continue;
		mapfoods[u->map_goods_y[i]][u->map_goods_x[i]][0]=0;
		mapfoods[u->map_goods_y[i]][u->map_goods_x[i]][1]=i;
	}
	if (u->who!=u->m[2]) mapfoods[u->m[0]][u->m[1]][0]=inf; //己方去吃的地方不再去
	//mapfoods[u->y[!u->who]][u->x[!u->who]][0]=inf;
	/////////////////////////////////////////////
	bfs[num][0]=u->y[u->who]; bfs[num++][1]=u->x[u->who];
	maptmp[u->y[u->who]][u->x[u->who]]=0;
	for ( int i=1; i<num; i++) {  //bfs将每个点到达需要的步数求出、、
		if (u->who==0) for ( int j=1; j<=4; j++) {
			int tmpy = bfs[i][0] + fx[j][0];
	        int tmpx = bfs[i][1] + fx[j][1];
	        if (tmpy >= 0 && tmpy < boderY && tmpx >= 0 && tmpx < boderX && u->mapInfo[tmpy][tmpx] == 1 && maptmp[tmpy][tmpx] == inf) {
				maptmp[tmpy][tmpx] = maptmp[bfs[i][0]][bfs[i][1]] + 1;
		        bfs[num][0] = tmpy;
		        bfs[num++][1] = tmpx;
			    /////////////////////////// 判断是否为物品 如果是判断到达后物品是否已经刷新出来 是的话come
			    if ((mapfoods[tmpy][tmpx][0]==0) && (u->map_goods_time[mapfoods[tmpy][tmpx][1]]*100+maptmp[tmpy][tmpx]*timefoodoff>=u->t)){
				    x=tmpx; y=tmpy; break;
				}
				
				else{
					if ((mapfoods[tmpy][tmpx][0]==0)&& u->map_goods_time[mapfoods[tmpy][tmpx][1]]*100+maptmp[tmpy][tmpx]*timefoodoff > first){
						first=u->map_goods_time[mapfoods[tmpy][tmpx][1]]*100+maptmp[tmpy][tmpx]*timefoodoff;
						xx=tmpx; yy=tmpy;
					}
				
				}
				
			    //////////////////////////////////////////
			}
        }else for ( int j=4; j>0; j--) {
			int tmpy = bfs[i][0] + fx[j][0];
	        int tmpx = bfs[i][1] + fx[j][1];
	        if (tmpy >= 0 && tmpy < boderY && tmpx >= 0 && tmpx < boderX && u->mapInfo[tmpy][tmpx] == 1 && maptmp[tmpy][tmpx] == inf) {
				maptmp[tmpy][tmpx] = maptmp[bfs[i][0]][bfs[i][1]] + 1;
		        bfs[num][0] = tmpy;
		        bfs[num++][1] = tmpx;
			    /////////////////////////// 判断是否为物品 如果是判断到达后物品是否已经刷新出来 是的话come
			    if ((mapfoods[tmpy][tmpx][0]==0) && (u->map_goods_time[mapfoods[tmpy][tmpx][1]]*100+maptmp[tmpy][tmpx]*timefoodoff>=u->t)){
				    x=tmpx; y=tmpy; break;
				}
				
				else{
					if ((mapfoods[tmpy][tmpx][0]==0)&& u->map_goods_time[mapfoods[tmpy][tmpx][1]]*100+maptmp[tmpy][tmpx]*timefoodoff > first){
						first=u->map_goods_time[mapfoods[tmpy][tmpx][1]]*100+maptmp[tmpy][tmpx]*timefoodoff;
						xx=tmpx; yy=tmpy;
					}
				}
				
			    //////////////////////////////////////////
			}
        }
		if (x!=inf) break;
	}
	
	if (mapfoods[u->y[u->who]][u->x[u->who]][0]==0 && u->map_goods_time[mapfoods[u->y[u->who]][u->x[u->who]][1]]*100>first) {
		xx=u->x[u->who]; yy=u->y[u->who];
	}
	
	if (x!=inf){ tmp=goto_des(x,y,u); u->m[2]=u->who; u->m[0]=y; u->m[1]=x;}
	else if (goodstype==inf && xx!=0) { tmp=goto_des(xx,yy,u); u->m[2]=u->who; u->m[0]=yy; u->m[1]=xx; }
	return tmp;
}
bool checkeskill(User *u)
{
	if (u->ey[0]==u->m[52+hashh[u->who]] && u->ex[0]==u->m[53+hashh[u->who]] &&
		u->ey[1]==u->m[56+hashh[u->who]] && u->ex[1]==u->m[57+hashh[u->who]] && 
		!(u->ey[0]==0 && u->ex[0]==0) && !(u->ey[1]==0 && u->ex[1]==0)){
			u->m[58+hashh[u->who]]++;
	}
	else {
		u->m[52+hashh[u->who]]=u->ey[0]; u->m[53+hashh[u->who]]=u->ex[0];
		u->m[56+hashh[u->who]]=u->ey[1]; u->m[57+hashh[u->who]]=u->ex[1];
		u->m[58+hashh[u->who]]=0;
		return false;
	}
	if (u->m[58+hashh[u->who]]>stoptimetestbskill) return true;
	return false;
}
Order findekill(User *u)
{
	Order tmp; tmp.act=STOP;
	tmp=curvekill(u); if (tmp.act==ATC) return tmp;
	if (!(u->ey[u->who]==0 && u->ex[u->who]==0)) tmp=goto_des(u->ex[u->who],u->ey[u->who],u);
    return tmp;
}
int dfs(User *u,int y,int x)
{
	int bfs[boderY*boderX][2];
	int maptmp[boderY][boderX];
	fill(maptmp[0],maptmp[boderY],inf);
	int deep=0;
	bfs[0][0]=y; bfs[0][1]=x; maptmp[y][x]=0;
	for (int i=0;i<=deep;i++)
		for (int j=1;j<=4;j++){
			int tmpy=bfs[i][0]+fx[j][0];
			int tmpx=bfs[i][1]+fx[j][1];
			if (tmpx<boderX && tmpx>=0 && tmpy<boderY && tmpy>=0 && maptmp[tmpy][tmpx]==inf){
				deep++;
				bfs[deep][0]=tmpy;
				bfs[deep][1]=tmpx;
				maptmp[tmpy][tmpx]=0;
				if (u->mapInfo[tmpy][tmpx]==1){
					return tmpy*100+tmpx;
				}
			}
		}
	return 0;
}
Order escape(User *u)
{
	Order tmp; tmp.act=STOP;
    int con=dfs(u,0,0); int conner1y=con/100; int conner1x=con%100;
	con=dfs(u,0,boderX-1); int conner2y=con/100; int conner2x=con%100;
	con=dfs(u,boderY-1,0); int conner3y=con/100; int conner3x=con%100;
	con=dfs(u,boderY-1,boderX-1); int conner4y=con/100; int conner4x=con%100;
	if (u->y[u->who]<boderY/2 && u->x[u->who]<boderX/2) tmp=goto_des(conner1x,conner1y,u);
	if (u->y[u->who]<boderY/2 && u->x[u->who]>=boderX/2) tmp=goto_des(conner2x,conner2y,u);
	if (u->y[u->who]>=boderY/2 && u->x[u->who]<boderX/2) tmp=goto_des(conner3x,conner3y,u);
	if (u->y[u->who]>=boderY/2 && u->x[u->who]>=boderX/2) tmp=goto_des(conner4x,conner4y,u);
	return tmp;
}
Order bigskill_0(User *u)
{
	Order tmp; tmp.act=STOP;
	switch (u->who){
	case 0: 
		if (u->m[40]<=10000 && u->m[40]-u->t>800 && u->m[41]-u->t>=5000) {tmp.act=KILL; u->m[41]=u->t; return tmp;}
		break;
	case 1: 
		if (u->m[40]-u->t>=5000 && !checkeskillans && u->t<=timebskill1start) { tmp.act=KILL; u->m[40]=u->t; return tmp; }
		break;
	}
	if (u->m[40]-u->t<=1000 && u->m[40]<=10000) tmp=findekill(u);  //如果敌方没释放 我方释放 寻找杀之
	if (checkeskillans  && (u->m[40]>10000 || (u->m[40]<=10000 && u->m[40]-u->t>1000))) tmp=escape(u); //如果敌方释放了  我没释放 果断跑之
	return tmp;
}
bool checkeskill2(User *u)
{
	if (u->ey[0]==0 && u->ey[1]==0 && u->ex[0]==0 && u->ex[1]==0 && u->y[0]==0 && u->x[0]==0 && u->y[1]==0 && u->x[1]==0) return true;
	return false;
}
bool alive(User *u)
{
	if (u->x[0]==0 && u->y[0]==0) return false;
	if (u->x[1]==0 && u->y[1]==0) return false;
	return true;
}
Order bigskill_1(User *u)
{
	Order tmp; tmp.act=STOP;
	switch (u->who){
	case 0: if (!checkeskill2(u) && u->b_skill_cd[0]==0) tmp.act=KILL; break;
	case 1: if (u->b_skill_cd[1]==0 && u->m[103]==inf && u->t>4000 && alive(u)) tmp.act=KILL;
			if (u->t<2000 && !(u->y[0]==0 && u->x[0]==0 && u->y[1]==0 && u->x[1]==0) && !(u->ey[0]==u->ey[1] && u->ex[0]==u->ex[1])
				&& u->b_skill_cd[1]==0) tmp.act=KILL;
			break;
	}
	if (u->b_skill_cd[0]>4000) tmp=findekill(u);
	return tmp;
}
bool checkspace(User *u)
{
	int all=0;
	for (int i=0;i<boderY;i++)
		for (int j=0;j<boderX;j++) 
			if (u->mapInfo[i][j]==LAND) all++;
	int bfs[boderY*boderX][2];
	int maptmp[boderY][boderX];
	fill(maptmp[0],maptmp[boderY],inf);
	int deep=0;
	bfs[0][0]=u->y[u->who];
	bfs[0][1]=u->x[u->who];
	maptmp[u->y[u->who]][u->x[u->who]]=0;
	mapspace[u->y[u->who]][u->x[u->who]]=4;
	for (int i=0;i<=deep;i++)
		for (int j=1;j<=4;j++){
			int tmpy=bfs[i][0]+fx[j][0];
			int tmpx=bfs[i][1]+fx[j][1];
			if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND && maptmp[tmpy][tmpx]==inf){
				deep++;
				bfs[deep][0]=tmpy;
				bfs[deep][1]=tmpx;
				maptmp[tmpy][tmpx]=0;
				mapspace[tmpy][tmpx]=4;
			}
		}
	if (deep-2>all/2) return true;
	return false;
}
Order cutthistree(User *u,int ii,int jj)
{
	Order tmp; tmp.act=STOP;

	for (int i=1;i<=4;i++){
		int tmpy=ii+fx[i][0];
		int tmpx=jj+fx[i][1];
		if (tmpx>=0 && tmpx<boderX && tmpy>=0 && tmpy<boderY && mapspace[tmpy][tmpx]==4){
			if (tmpy==u->y[u->who] && tmpx==u->x[u->who]){
				tmp.act=TWO; 
				if (i==LEFT) tmp.dir[0]=RIGHT; 
				if (i==RIGHT) tmp.dir[0]=LEFT; 
				if (i==UP) tmp.dir[0]=DOWN; 
				if (i==DOWN) tmp.dir[0]=UP; 
				return tmp;
			}
			tmp=goto_des(tmpx,tmpy,u); return tmp; 
		}
	}
}
Order cuttree(User *u)
{
	Order tmp; tmp.act=STOP;
	for (int i=0;i<boderY;i++)
		for (int j=0;j<boderX;j++)
			if (mapspace[i][j]==TREE){
				if (i-1>=0 && i+1<boderY && mapspace[i-1][j]+mapspace[i+1][j]==5) {tmp=cutthistree(u,i,j); return tmp;}
				if (j-1>=0 && j+1<boderX && mapspace[i][j-1]+mapspace[i][j+1]==5) {tmp=cutthistree(u,i,j); return tmp;}
			}
	return tmp;
}
Order movetree(User *u)
{
	Order tmp; tmp.act=STOP;
	if (u->id==0 || u->id==2){
		if (checkeskillans) return tmp;
	}else
	{
		if (checkeskill2(u)) return tmp;
	}
	if (u->ey[0]==0 && u->ex[0]==0) return tmp;
	if (u->ey[1]==0 && u->ex[1]==0) return tmp;
	int bfs[boderY*boderX][2];
	int deep=0;
	int maptmp[boderY][boderX];
	fill(maptmp[0],maptmp[boderY],inf);
	int enumm=0;
	bfs[0][0]=u->y[u->who]; bfs[0][1]=u->x[u->who];
	maptmp[u->y[u->who]][u->x[u->who]]=0;
	for (int i=0;i<=deep;i++)
		for (int j=1;j<=4;j++){
			int tmpy=bfs[i][0]+fx[j][0];
			int tmpx=bfs[i][1]+fx[j][1];
			if (tmpy<boderY && tmpy>=0 && tmpx<boderX && tmpx>=0 && u->mapInfo[tmpy][tmpx]==LAND && maptmp[tmpy][tmpx]==inf){
				deep++;
				bfs[deep][0]=tmpy;
				bfs[deep][1]=tmpx;
				maptmp[tmpy][tmpx]=0;
				if (tmpy==u->ey[0] && tmpx==u->ex[0]) enumm++;
				if (tmpy==u->ey[1] && tmpx==u->ex[1]) enumm++;
			}
		}
	if (enumm==2) return tmp; 
	if (checkspace(u)) return tmp;
	tmp=cuttree(u);
	return tmp;
}
int searchplace(User *u)
{
	int num=0;
	for (int i=boderY-1;i>=0;i--){
		for (int j=boderX-1;j>=0;j--)
			if (u->mapInfo[i][j]==LAND){
				num=0;
				for (int k=1;k<=4;k++){
					int tmpy=i+fx[k][0];
					int tmpx=j+fx[k][1];
					if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]!=TREE) num++;
				}
				if (num==2){
					u->m[100]=i;
					u->m[101]=j;
					break;
				}
			}
		if (num==2)break;
	}
	return 0;
}
Order planttree(User *u)
{
	Order tmp; tmp.act=STOP;
	int tmpy,tmpx,dir=inf;
	for (int ii=1;ii<=4;ii++){
		int i;
		if (u->who==0) i=ii; else i=5-ii;
		tmpy=u->m[100]+fx[i][0];
		tmpx=u->m[101]+fx[i][1];
		if (tmpy>=0 && tmpy<boderY && tmpx>=0 && tmpx<boderX && u->mapInfo[tmpy][tmpx]==LAND){dir=i; break;}
	}
	if (dir==inf) return tmp;
	if (u->s_skill_cd[u->who]==0) tmp=goto_des(u->m[101],u->m[100],u);
	if (tmp.act!=STOP) return tmp;
	if (!(u->y[u->who]==u->m[100] && u->x[u->who]==u->m[101])) return tmp;
	if (u->who==0) u->m[102]=inf; else u->m[103]=inf;
	//if (u->id==3 && u->m[102]!=inf) return tmp;
	tmp.act=ONE;
	tmp.dir[0]=dir;
	return tmp;
}
bool success(User *u)
{
	if (u->y[u->who]==u->ey[0] && u->ey[0]==u->ey[1] && u->x[u->who]==u->ex[0] && u->ex[0]==u->ex[1]) return true;
	return false;
}
extern "C" AI_USER_API Order get_order(User *u) {
	Order tmp; tmp.act = STOP;
	////////////////////////////////////////////////////// 最后的修正
	if (u->id==0 && u->eb_skill_cd[1]>=4995 && u->magic_len[u->who]>=12){
		tmp.act=44; return tmp;
	}
	/////////////////////////////////////////////////////最后的修正
	checkemagiclen(u);
	checkeskillans=checkeskill(u);
	fill(killmap[0],killmap[boderY],inf);

	//escape
	
	for (int i=0;i<=1;i++){
		can[i]=false;
		killmap[u->y[u->who]][u->x[u->who]]=inf;
		findkillme(u,i,u->y[u->who],u->x[u->who]);
		if (killmap[u->y[u->who]][u->x[u->who]]==0) can[i]=true;		
	}


	if (u->magic_num[!u->who]!=0)
		for (int i=0;i<u->magic_num[!u->who];i++)
			killmap[u->magic_y[!u->who][i]][u->magic_x[!u->who][i]]=0;
	for (int i=0;i<u->magic_num[u->who];i++) killmap[u->magic_y[u->who][i]][u->magic_x[u->who][i]]=0;//不走自己魔法的地方

	if (can[0] || can[1]) tmp=goto_des2(u,u->y[u->who],u->x[u->who]);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	if (tmp.act!= STOP) return tmp;
	////////////////////////////////////////////plant tree
	

	if (u->t>9950) {u->m[102]=0; u->m[103]=0;}
	if (u->m[100]==0 && u->m[101]==0) searchplace(u); //寻找种树中心点
	if (!checkeskill2(u) && u->id==1 && u->b_skill_cd[1]==0 && u->s_skill_cd[0]==0 && u->m[102]==0 && u->t<9950) tmp=planttree(u);
	if (u->id==3 && u->b_skill_cd[1]==0 && u->t<9900 && u->m[103]==0 && u->m[102]==inf) tmp=planttree(u);

	if (tmp.act!=STOP) return tmp;
	//////102 103为技能标记
	
	/////////////////////////////////////////////cut tree
	
	for (int i=0;i<boderY;i++)
		for (int j=0;j<boderX;j++)
			mapspace[i][j]=u->mapInfo[i][j];
	
	if (u->s_skill_cd[u->who]==0){
		if (u->id==3 && success(u)) goto Loop;
		tmp=movetree(u);
		if (tmp.act!=STOP) return tmp;
	}

	Loop:
	
	//////////////////////////////////////BIG skill
	
	if (u->m[40]==0) u->m[40]=20000;
	if (u->m[41]==0) u->m[41]=20000;
	switch (u->id){
	case 0: tmp=bigskill_0(u); break;
	case 2: tmp=bigskill_0(u); break;
	case 1: tmp=bigskill_1(u); break;
	case 3: tmp=bigskill_1(u); break;
	}
	u->m[59]=u->b_skill_cd[1];
	if (tmp.act!=STOP) return tmp;
	
	///////////////////////////////////////
	
	

	tmp=curvekill(u); if (tmp.act==ATC) return tmp; 
//	finddangerway;///计算狭窄通道、、、
	if (u->t>2000){
		tmp=movefoods(u,BLUE); if (tmp.act!=STOP)return tmp;
		tmp=movefoods(u,BOMB); if (tmp.act!=STOP)return tmp;
		tmp=movefoods(u,BOOK); if (tmp.act!=STOP)return tmp;
	}else
	{
		tmp=movefoods(u,BOMB); if (tmp.act!=STOP)return tmp;
		tmp=movefoods(u,BLUE); if (tmp.act!=STOP)return tmp;
		tmp=movefoods(u,BOOK); if (tmp.act!=STOP)return tmp;
	}
	
	//tmp=movefoods(u,inf);
	return tmp;
}