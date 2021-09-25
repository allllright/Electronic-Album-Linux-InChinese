#include <stdio.h>
#include <sys/types.h>//打开创建函数头文件
#include <sys/stat.h>//打开创建函数头文件
#include <fcntl.h>//打开创建函数头文件
#include <errno.h>
#include <unistd.h>//关闭、读、写数据函数头文件
#include <linux/input.h>//输入结构体头文件
#include <sys/mman.h>//内存映射头文件

int lcd_fd;
int *plcd ;
//以下主函数是210获取触摸屏坐标的代码
struct piont
{
	int x,y;
};
	//定义触摸屏函数
int get_touch()		
{
	struct piont start,end;
	start.x = start.y = end.x = end.y = 0;
	
	//第一步：打开触摸屏 
	int input_fd = open("/dev/event0",O_RDWR);
	if(input_fd < 0)
	{
		perror("open input lcd error");
		return -1;
	}
	//第二步：读出输入的事件 
	
	struct input_event data;
	int temp_x,temp_y;
	temp_x = temp_y = 0;
	while(1) //对于触摸屏而言有两种操作 即 滑动和点击 无论滑动和点击都具有持续性
	{
		read(input_fd,&data,sizeof(data));
		//分析数据
		if(data.type == EV_ABS)	//code就表示坐标轴（绝对坐标）
		{
			if(data.code == ABS_X)
			{
				//给x轴
				temp_x = data.value;

			}
			else if(data.code == ABS_Y)
			{
				//值给Y
				temp_y = data.value;

			}
			else if(data.code == ABS_PRESSURE) //表示触摸屏的键值（压力键）
			{
				if(data.value > 0) 		//相应的x/y轴的坐标值
				{
					if(start.x ==0 && start.y  == 0)
					{
						start.x = temp_x;
						start.y = temp_y;
					}
				}
				else if(data.value == 0)	//相应的x/y轴的坐标值
				{
					end.x = temp_x;
					end.y = temp_y;
					break;
				}
			
			}
		}
	}
	//第三步：处理输入的数据
	printf("start(%d,%d),end(%d,%d)\n",start.x,start.y,end.x,end.y);
	//第四步：返回结果
	int a = end.x-start.x;
	int b = end.y-start.y;
	if((a<b)&&(a<b*(-1)))	//a<b 且 a<-b
	{
		return 1;	//左滑

	}
	else if((a>b)&&(a>b*(-1))) //a>b 且 a>-b
	{
		return 2;	//右滑
	}
	else if((a<b)&&(a>b*(-1))) //a<b 且 a>-b
	{
		return 3;	//上滑
	}
	else if((a>b)&&(a<b*(-1))) //a>b 且 a<-b
	{	
		return 4;	//下滑
	}
}

	//打开屏幕 函数
int lcd_init(void)
{
	lcd_fd = open("/dev/fb0",O_RDWR);	//只读模式
	if(lcd_fd < 0)
	{
		perror("open lcd error");//标准出错函数 可以打印出错信息
		return -1;
	}
			//映射屏幕
	plcd = (int*)mmap(NULL,480*800*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
	if(plcd == NULL)	//指针为空
	{
		perror("mmap lcd error");//标准出错函数 可以打印出错信息
		return -1;
	}
}
	//关闭屏幕函数
void lcd_uninit(void)
{
	munmap(plcd,480*800*4);	//解映射
	close(lcd_fd);	//关屏幕
}

	//显示图片函数
int show_bmp(char* pathname)
{
	//第一步：打开图片
	int pic_fd = open(pathname,O_RDONLY);
	if(pic_fd <0)
	{
		perror("open pic error");//标准出错函数 可以打印出错信息
		return -1;
	}
	
	//第二步：p偏移54字节文件头 
	lseek(pic_fd,54,SEEK_SET);
	//第三步：读取图片数据
	char pic_buf[480*800*3] = {0};
	int ret = read(pic_fd,pic_buf,480*800*3);
	if(ret < 480*800*3)
	{
		printf("read pic error:pic size fail\n");
		return -1;
	}
	//第四步：处理数据
	char a,r,g,b;
	int i,j,x = 0;
	for(i = 0;i <480;i++)
	{
		for(j = 0;j<800;j++)
		{
			a = 0x00;
			b = pic_buf[x++]; 
			g = pic_buf[x++];
			r = pic_buf[x++]; 
			draw_piont(479-i,j,a<<24|r<<16|g<<8|b);
		}
	}
	
	
}
	//主函数
void main()
{
	char *str[3]={"./11.bmp","./12.bmp","./13.bmp"};		//定义指针数组
	int xx=0;		//取整xx=0
	lcd_init();		//打开屏幕
			//
			printf("hhh183\n");
		while(1)
	{
		show_bmp(str[xx]);			//根据返回的xx的值选择图片显示
		int ret = get_touch();		//get_touch函数的返回值给ret
		if(ret == 1|| ret == 3)		//判断
		{
			xx++;
			if(xx>2)				//判断
			{
				xx = 0;
			}
		}
		else if(ret == 2|| ret == 4)		//判断
		{
			xx--;
			if(xx<0)				//判断
			{
				xx = 2; 
			}
		}
	
	}
	lcd_uninit();					//关闭屏幕

}


	//画点函数
void draw_piont(int i,int j,int color)
{
	*(plcd+i*800+j) = color;
}
	//画块函数
void draw_block(int x0,int y0,int len,int depth,int color)
{
	int i,j;
	for(i = y0;i<y0+depth;i++)
	{
		for(j = x0;j<x0+len;j++)
		{
			draw_piont(i,j,color);
		}
	}
}
