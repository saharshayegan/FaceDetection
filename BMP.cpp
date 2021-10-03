#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
using namespace std;

const int maxn=1000;
float rgb[maxn][maxn][3];
float HSL[maxn][maxn];
int detect[maxn][maxn];
unsigned char data[maxn][maxn * 3];  // allocate 3 bytes per pixel
unsigned char info[54];
int size=1,height,width;
int padding=0;
int numcomp=2;    
///////////////////////////////////

class component
{
    public :      
    vector < pair<int,int> > pixel;
    float hei,wid;
    float surface;
    int Min_i,Max_i,Min_j,Max_j;
    void find_bound();    
    void set_value();
    bool is_face();
    void detect_face();
}face[maxn];

void component::detect_face()
{
    if(is_face())
    {
        for(int i=Min_i;i<=Max_i;i++)
        {
            data[i][3*Max_j]=0;
            data[i][3*Max_j+1]=0;
            data[i][3*Max_j+2]=255;
            data[i][3*Min_j]=0;
            data[i][3*Min_j+1]=0;
            data[i][3*Min_j+2]=255;
        }
        for(int j=Min_j;j<=Max_j;j++)
        {
            data[Max_i][3*j]=0;
            data[Max_i][3*j+1]=0;
            data[Max_i][3*j+2]=255;
            data[Min_i][3*j]=0;
            data[Min_i][3*j+1]=0;
            data[Min_i][3*j+2]=255;
        }
    }
}

bool component::is_face()
{
    set_value();
    if(surface < 1500)
        return false;
    float per_sur=(float)pixel.size() / surface;
    if(per_sur > 0.9 or per_sur < 0.5)
        return false;
    float golden_ratio = hei/wid;
    if(1.2>golden_ratio or golden_ratio > 2.55)
        return false;
    return true;
}

void component::set_value()
{
    find_bound();
    hei=Max_i-Min_i;
    wid=Max_j-Min_j;
    surface=hei*wid;
}

void component::find_bound()
{
    float x=maxn,k=0,y=maxn,l=0;
    for(int i=0;i<(int)pixel.size();i++)
    {
        if(x>pixel[i].first)
        {
            Min_i=i;
            x=pixel[i].first;
        }
        if(k<pixel[i].first)
        {
            Max_i=i;
            k=pixel[i].first;
        }
        if(y>pixel[i].second)
        {
            Min_j=i;
            y=pixel[i].second;
        }
        if(l<pixel[i].second)
        {
            Max_j=i;
            l=pixel[i].second;
        }
    }
    Min_i=pixel[Min_i].first;
    Max_i=pixel[Max_i].first;
    Min_j=pixel[Min_j].second;
    Max_j=pixel[Max_j].second;    
}
    
void readBMP(char* filename)
{
    int i;
    FILE* f = fopen(filename, "rb");
    fread(info, sizeof(unsigned char), 54, f);  // read the 54-byte header

    // extract image height and width from header :
    width = *(int*)&info[18];
    height = *(int*)&info[22];
    size = 3 * width * height;
    padding=(4-(3*width%4))%4;
    
    for(int i=0;i<height;i++)
    {
        fread(data[i],sizeof(unsigned char),width*3,f);
        fseek(f,padding,SEEK_CUR);
    }
    fclose(f);
    
    for(i = 0; i < height; i++)
        for(int j=0;j<width;j++)
        {
            rgb[i][j][2]=data[i][3*j];
            rgb[i][j][1]=data[i][3*j+1];
            rgb[i][j][0]=data[i][3*j+2];
        }
}

//RGB TO HSL
void rgbtohsl()
{
     for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
        {
            float r1=rgb[i][j][0]/255;
            float g1=rgb[i][j][1]/255;
            float b1=rgb[i][j][2]/255;
            float maxv = max(max(r1, g1), b1);
            float minv = min(min(r1, g1), b1);
            float h = 0, s = 0, d = maxv - minv;
            float l = (maxv + minv) / 2;
            if(minv == maxv)
                l*=240;
            if (maxv != minv)
            {
                s = (l > 0.5 ? d / (2 - maxv - minv) : d / (maxv + minv));
                if (maxv == r1) { h = (g1 - b1) / d ; }
                else if (maxv == g1) { h = (b1 - r1) / d + 2; }
                else if (maxv == b1) { h = (r1 - g1) / d + 4; }
               if(h < 0)
                h+=6;
               h *= 40;
            }  
            HSL[i][j]=h;
        }        
}

//REJECT 
void reject()
{
        for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
        {
            float r=rgb[i][j][0]/(rgb[i][j][0]+rgb[i][j][1]+rgb[i][j][2]);
            float g=rgb[i][j][1]/(rgb[i][j][0]+rgb[i][j][1]+rgb[i][j][2]);
            float f1=(r*r*(-1.376))+((1.0743)*r)+(0.2);
            float f2=(r*r*(-0.776))+((0.5601)*r)+(0.18);
            float w=((r-0.33)*(r-0.33)) + ((g-0.33)*(g-0.33)); 
            if(!(g < f1 and g > f2 and w >0.001))
            {
                detect[i][j]=0;
 /*               data[i][3*j]=0;
                data[i][3*j+1]=0;
               data[i][3*j+2]=0;*/
            }
         }
        
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
            if(! (HSL[i][j] < 20 or HSL[i][j] >=239))
            {
                detect[i][j]=0;
                /*data[i][3*j]=0;
                data[i][3*j+1]=0;
                data[i][3*j+2]=0;*/
            }
   
}

void set()
{    
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
            detect[i][j]=1;
   /* for(int i=0;i<height;i++)
        for(int j=0;j<width*3;j++)
            data[i][j]=255;
*/
}

void test()
{    
    FILE* f = fopen("ans.bmp", "wb");
    int b = 0;
    fwrite(info, sizeof(unsigned char), 54, f);
    for(int i=0;i<height;i++)
    {    
        fwrite(data[i],sizeof(unsigned char),width*3,f);
        fwrite(&b,sizeof(unsigned char),padding,f);
    }   
    fclose(f);
}

int mv[8][2]={{-1,-1},{-1,0},{0,-1},{1,-1},{-1,1},{0,1},{1,0},{1,1}};

void dfs(int x,int y)
{
    face[numcomp].pixel.push_back(make_pair(x,y));
    detect[x][y]=numcomp;  
  //  cout<<x<<"-"<<y<<" : "<<detect[x][y]<<" va "<<numcomp<<" || ";
    for(int i=0;i<8;i++)
    {
        int kx=x+mv[i][0],ky=y+mv[i][1];
        if(detect[kx][ky]==1 and kx<height and ky<width and kx>=0 and ky>=0)
            dfs(kx,ky);
    }      
}

void find_comp()
{
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
        {
            if(detect[i][j]==1)
            {
                dfs(i,j);
                numcomp++;
            }
        }
}

void noise()
{
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)    
        {
            int s=0;
            for(int k=0;k<8;k++)
            {
                int x=i+mv[k][0];
                int y=j+mv[k][1];
                if(detect[x][y]==1)
                    s++;
            }
            if(s<=3)
                detect[i][j]=0;
        }
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
            if(detect[i][j]==1)
            {
                int s=0;
                while(detect[i][j]==1)
                {
                    s++;
                    j++;
                }
                if(s<3)
                    for(int k=j-s;k<j;k++)
                        detect[i][k]=0;
            }    
}
int main()
{
    //read bmp
    char a[100] = "2.bmp";
    readBMP(a);
    set();
    rgbtohsl();
    reject();
    noise();
    find_comp();
    for(int i=2;i<numcomp;i++)
        face[i].detect_face();
    test();
   // system("pause");
    
}
