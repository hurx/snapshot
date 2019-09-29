#include "snap.h"
int main(int argc, char* argv[]){
	//char* file_path = "LOL.mp4";//待解码文件
	char* file_path = "http://1253039488.vod2.myqcloud.com/d8c9fd32vodtransgzp1253039488/1f8f10735285890794368016706/v.f10.mp4";//待解码文件
	char* out_pic = "snapshot.jpg";
	double shot_time = 5.0;
	SnapShot(file_path,out_pic,shot_time);
	return 0;
}


