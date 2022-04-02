//
// Created by ly on 2022/3/29.
//

#include "isp_demosaic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

void bubble_sort(int16* data,uint8 len){
    for(uint8 i =0;i<len-1;i++){
        for(uint8 j=0;j<len-i-1;j++){
            if(data[j]>data[j+1]){
                int16 temp =data[j];
                data[j]=data[j+1];
                data[j+1]=temp;
            }
        }
    }
    return;
}
void median_filter(int16** In_img,uint16 In_width,uint16 In_height,const board_info boardInfo){
    uint16 top = boardInfo.top;
    uint16 bottom = boardInfo.bottom;
    uint16 left = boardInfo.left;
    uint16 right = boardInfo.right;

    for(uint16 v = top;v < In_height-bottom;v++){
        for(uint16 h = left;h < In_width-right;h++){
            int16 value_in_kernel[9] ={ In_img[v-1][h-1],In_img[v-1][h],    In_img[v-1][h+1],//
                                        In_img[v][h-1],  In_img[v][h],      In_img[v][h+1],//
                                        In_img[v+1][h-1],In_img[v+1][h],    In_img[v+1][h+1]};
            bubble_sort(value_in_kernel,9);
            In_img[v][h] = value_in_kernel[4];
      }
    }
    return;
}
void remove_border(uint16** In_img,uint16** Out_img,uint16 In_width,uint16 In_height,const board_info boardInfo){

    uint16 top = boardInfo.top;
    uint16 bottom = boardInfo.bottom;
    uint16 left = boardInfo.left;
    uint16 right = boardInfo.right;
    for(uint16 v = top;v < In_height-bottom;v++){
        for(uint16 h = left;h < In_width-right;h++){
            Out_img[v-top][(h-left)*3+0]=In_img[v][3*h+0];
            Out_img[v-top][(h-left)*3+1]=In_img[v][3*h+1];
            Out_img[v-top][(h-left)*3+2]=In_img[v][3*h+2];
        }
    }
    return;
}
void reshape(uint16* Input,uint16**Output,uint16 img_width,uint16 img_height){

        uint16 m = img_width;
        uint16 n = img_height;
        for(uint16 v = 0;v <n;v++){
            for(uint16 h=0;h<m;h++){
                Output[v][h]=Input[v*m+h];
            }
        }
        return;
    }
void MakeBorder( uint16* Input, uint16* Output, const board_info boardInfo,const raw_info input_img_info){

    uint16 input_width = input_img_info.u16ImgWidth;
    uint16 input_height = input_img_info.u16ImgHeight;
    uint16 top = boardInfo.top;
    uint16 bottom = boardInfo.bottom;
    uint16 left = boardInfo.left;
    uint16 right = boardInfo.right;
    //copy input part
    for(uint16 y = top;y < input_height + top;y++){
        for(uint16 x = left ;x < input_width + left;x++){
            Output[y*(input_width+left+right)+x] = Input[(y-top)*input_width+x-left];
        }
    }
    //padding top
    memcpy(&Output[0],&Output[top*(input_width+left+right)],top*(input_width+left+right)*sizeof(uint16));
    //padding bottom
    memcpy(&Output[(input_height + top)*(input_width+left+right)],&Output[(input_height+top-bottom)*(input_width+left+right)],bottom*(input_width+left+right)*sizeof(uint16));


    //padding left
    for(uint16 y1 = 0;y1< input_height+top+bottom;y1++){
        for(uint16 x1 = 0;x1 < left;x1++){
            Output[y1*(input_width+left+right)+x1] = Output[y1*(input_width+left+right)+left+x1];
        }
    }

    //padding right
    for(uint16 y2 = 0;y2< input_height+top+bottom;y2++){
        for(uint16 x2 = 0;x2 < right;x2++){
            Output[y2*(input_width+left+right)+input_width+left+x2] = Output[y2*(input_width+left+right)+input_width+left-right+x2];
        }
    }


    return;
}
#if 1
 void bilinear_demosaic(uint16** In_img,uint16** Out_img,uint16 In_width,uint16 In_height){

     uint8 K_size =3;
     //const uint8 K_R[3][3]= {{1, 2, 1},{2, 4, 2},{1, 2, 1}};
     //const uint8 K_G[3][3]= {{0, 1, 0},{1, 4, 1},{0, 1, 0}};
     //const uint8 K_B[3][3]= {{1, 2, 1},{2, 4, 2},{1, 2, 1}};
    uint8 start_x = K_size/2;// 3/2
    uint8 start_y = K_size/2;// 3/2

    //first_pixel B //bayer_pattern_BGGR
    //fill in Out_img from In_img
     for(uint16 j = 0;j<In_height;j++){
         for(uint16 i = 0;i<In_width;i++){
             if(BPBG == bayerPattLUT[3][j & 0x1][i & 0x1]){
                 Out_img[j][i*3+2] = In_img[j][i];
                 Out_img[j][i*3+0] = 0;
                 Out_img[j][i*3+1] = 0;
             }
             else if((BPGB == bayerPattLUT[3][j & 0x1][i & 0x1]) || (BPGR == bayerPattLUT[3][j & 0x1][i & 0x1])){
                 Out_img[j][i*3+1] = In_img[j][i];
                 Out_img[j][i*3+0] = 0;
                 Out_img[j][i*3+2] = 0;
             }
             else{
                 Out_img[j][i*3+0] = In_img[j][i];
                 Out_img[j][i*3+1] = 0;
                 Out_img[j][i*3+2] = 0;

             }
         }
     }

     //bilinear interpolation RGB channel
     for(uint16 y = start_y;y<In_height-start_y;y++){
        for(uint16 x = start_x;x<In_width-start_x;x++){
            if(BPBG == bayerPattLUT[3][y & 0x1][x & 0x1]){
                Out_img[y][x*3+1] = (Out_img[y][(x-1)*3+1] + Out_img[y][(x+1)*3+1]+Out_img[y-1][x*3+1]+Out_img[y+1][x*3+1])>>2;
                Out_img[y][x*3+0] = (Out_img[y-1][(x-1)*3+0] + Out_img[y-1][(x+1)*3+0]+Out_img[y+1][(x-1)*3+0]+Out_img[y+1][(x+1)*3+0])>>2;
            }
            else if(BPRG == bayerPattLUT[3][y & 0x1][x & 0x1]){
                Out_img[y][x*3+1] = (Out_img[y][(x-1)*3+1] + Out_img[y][(x+1)*3+1]+Out_img[y-1][x*3+1]+Out_img[y+1][x*3+1])>>2;
                Out_img[y][x*3+2] = (Out_img[y-1][(x-1)*3+2] + Out_img[y-1][(x+1)*3+2]+Out_img[y+1][(x-1)*3+2]+Out_img[y+1][(x+1)*3+2])>>2;
            }
            else if(BPGB == bayerPattLUT[3][y & 0x1][x & 0x1]){
                Out_img[y][x*3+0] = (Out_img[y+1][x*3+0] + Out_img[y-1][x*3+0])>>1;
                Out_img[y][x*3+2] = (Out_img[y][(x-1)*3+2] + Out_img[y][(x+1)*3+2])>>1;
            }
            else if(BPGR == bayerPattLUT[3][y & 0x1][x & 0x1]){
                Out_img[y][x*3+0] = (Out_img[y][(x-1)*3+0] + Out_img[y][(x+1)*3+0])>>1;
                Out_img[y][x*3+2] = (Out_img[y-1][x*3+2] + Out_img[y+1][x*3+2])>>1;
            }
        }
    }
     return;
 }
#endif
void freeman_median_demosaic(uint16** In_img,uint16** bilinear_img,uint16** freeman_img,uint16 In_width,uint16 In_height,const board_info boardInfo){

        int16** D_rg = (int16**)malloc(In_height*sizeof(int16*));
        for(uint16 v1=0;v1 <In_height;v1++){
            D_rg[v1] = (int16*) malloc(In_width*sizeof(int16));
        }

        int16** D_bg = (int16**)malloc(In_height*sizeof(int16*));
        for(uint16 v2=0;v2 <In_height;v2++){
            D_bg[v2] = (int16*) malloc(In_width*sizeof(int16));
        }
        int16** D_rb = (int16**)malloc(In_height*sizeof(int16*));
        for(uint16 v3=0;v3 <In_height;v3++){
            D_rb[v3] = (int16*) malloc(In_width*sizeof(int16));
        }

        for(uint16 y1 =0;y1<In_height;y1++){
            for(uint16 x1=0;x1<In_width;x1++){
                D_rg[y1][x1] =bilinear_img[y1][3*x1+0]-bilinear_img[y1][3*x1+1];
                D_bg[y1][x1] =bilinear_img[y1][3*x1+2]-bilinear_img[y1][3*x1+1];
                D_rb[y1][x1] =bilinear_img[y1][3*x1+0]-bilinear_img[y1][3*x1+2];
//                if(BPRG == bayerPattLUT[3][y1 & 0x1][x1 & 0x1]){
//                    D_rg[y1][x1] = In_img[y1][x1]-bilinear_img[y1][3*x1+1];
//                }
//                else if(BPBG == bayerPattLUT[3][y1 & 0x1][x1 & 0x1]){
//                    D_bg[y1][x1] = In_img[y1][x1]-bilinear_img[y1][3*x1+1];
//                }
            }
        }
        median_filter(D_rg,In_width,In_height,boardInfo);
        median_filter(D_bg,In_width,In_height,boardInfo);
        median_filter(D_rb,In_width,In_height,boardInfo);
        //singleChannel2BMP(D_rg,In_width,In_height,"D:\\leetcode_project\\D_rg_img_rgb.bmp");
        //singleChannel2BMP(D_gb,In_width,In_height,"D:\\leetcode_project\\D_gb_img_rgb.bmp");
        //singleChannel2BMP(D_rb,In_width,In_height,"D:\\leetcode_project\\D_rb_img_rgb.bmp");
#if 1
        for(uint16 v =0;v<In_height;v++){
            for(uint16 h=0;h<In_width;h++){

                freeman_img[v][3*h+1] = bilinear_img[v][3*h+1];
                freeman_img[v][3*h+2] = CLIP3(D_bg[v][h]+freeman_img[v][3*h+1],0,4095);//B
                freeman_img[v][3*h+0] = CLIP3(D_rg[v][h] + freeman_img[v][3*h+1],0,4095);//R


          }
        }
#endif
        for(uint16 m=0;m < In_height;m++){
            free(D_rb[m]);
        }
        free(D_rb);

        for(uint16 i=0;i < In_height;i++){
            free(D_bg[i]);
        }
        free(D_bg);

        for(uint16 j=0;j < In_height;j++){
            free(D_rg[j]);
        }
        free(D_rg);
        return;
    }




int main(int argc,char**argv){
/*
    static u32 gamma_table[] = {
        #include "gamma_table.h"
    };
*/
    const raw_info raw_info = {0,0,1920,1080,12,BPRG};
    const board_info boardInfo ={1,1,1,1};
    uint16 raw_width = raw_info.u16ImgWidth;
    uint16 raw_height = raw_info.u16ImgHeight;
    uint16 top = boardInfo.top;
    uint16 bottom = boardInfo.bottom;
    uint16 left = boardInfo.left;
    uint16 right =boardInfo.right;
    char *raw_file = "D:\\all_isp\\test_img\\lab_1920x1080_12bits_RGGB_Linear.raw";
    char *save_raw = "D:\\leetcode_project\\raw_print_info.txt";
    char *save_Ext_raw = "D:\\leetcode_project\\Ext_raw_print_info.txt";
    char *save_reshape_raw = "D:\\leetcode_project\\reshape_raw_print_info.txt";
    char *save_RGB = "D:\\leetcode_project\\RGB_print_info.txt";

    char *save_BMP = "D:\\leetcode_project\\freeman_img_rgb.bmp";

    uint16* BayerImg = (uint16*)malloc(raw_width * raw_height * sizeof(uint16));
    if( NULL == BayerImg ){
        printf("BayerImg malloc fail!!!\n");
    }
    uint16* ext_BayerImg = (uint16*)malloc((raw_width+left+right) * (raw_height+top+bottom) * sizeof(uint16));
    if( NULL == ext_BayerImg ){
        printf("ext_BayerImg malloc fail!!!\n");
    }
    uint16** reshape_img = (uint16**)malloc((raw_height+top+bottom)*sizeof(uint16*));
    for(uint16 v1=0;v1 <(raw_height+top+bottom);v1++){
        reshape_img[v1] = (uint16*) malloc((raw_width+left+right)*sizeof(uint16));
    }


    uint16** bilinear_RGB = (uint16**)malloc((raw_height+top+bottom)*sizeof(uint16*));
    for(uint16 v2=0;v2 <(raw_height+top+bottom);v2++){
        bilinear_RGB[v2] = (uint16*) malloc(3*(raw_width+left+right)*sizeof(uint16));
    }

    uint16** freeman_RGB = (uint16**)malloc((raw_height+top+bottom)*sizeof(uint16*));
    for(uint16 v3=0;v3 <(raw_height+top+bottom);v3++){
        freeman_RGB[v3] = (uint16*) malloc(3*(raw_width+left+right)*sizeof(uint16));
    }

    uint16** rb_RGB = (uint16**)malloc(raw_height*sizeof(uint16*));
    for(uint16 v4=0;v4 <raw_height;v4++){
        rb_RGB[v4] = (uint16*) malloc(3*raw_width*sizeof(uint16));
    }

    read_BayerImg(raw_file,raw_height,raw_width,BayerImg);
    //print_raw_to_txt(BayerImg,1920,1080,save_raw);
    MakeBorder( BayerImg, ext_BayerImg, boardInfo,raw_info);
    //print_raw_to_txt(ext_BayerImg,1922,1082,save_Ext_raw);

    //reshape to matrix
    reshape(ext_BayerImg,reshape_img,raw_width+left+right,raw_height+top+bottom);
    //print_reshapeRAW_to_txt(reshape_img,raw_width+left+right,raw_height+top+bottom,save_reshape_raw);

    //bilinear demosaic
    bilinear_demosaic(reshape_img,bilinear_RGB,raw_width+left+right,raw_height+top+bottom);
    //print_RGB_to_txt(RGB_img,raw_width+left+right,raw_height+top+bottom,save_RGB);
    freeman_median_demosaic(reshape_img,bilinear_RGB,freeman_RGB,raw_width+left+right,raw_height+top+bottom,boardInfo);

    remove_border(freeman_RGB,rb_RGB,raw_width+left+right,raw_height+top+bottom,boardInfo);
    //print_RGB_to_txt(RGB_img,raw_width,raw_height,"D:\\leetcode_project\\remove_RGB_print_info.txt");
    RGB2BMP(rb_RGB,1920,1080,save_BMP);

    for(uint16 q=0;q < raw_height;q++){
        free(rb_RGB[q]);
    }
    free(rb_RGB);

    for(uint16 p=0; p <(raw_height+top+bottom);p++){
        free(freeman_RGB[p]);
    }
    free(freeman_RGB);

    for(uint16 j=0;j <(raw_height+top+bottom);j++){
        free(bilinear_RGB[j]);
    }
    free(bilinear_RGB);

    for(uint16 i=0;i <(raw_height+top+bottom);i++){
        free(reshape_img[i]);
    }
    free(reshape_img);

    free(ext_BayerImg);
    free(BayerImg);
    return 0;

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */