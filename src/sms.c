#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "retro.h"

int sms_write_rom(FILE *file,SDL_Surface *image,unsigned char *palette,int ncolor,int type)
{
    int casex,casey;
    int tiles[64];
    int i,l;
    int x,y,size = 0;
    int bin[128];
	int b,g,r;

	unsigned char *pixel = image->pixels;

    casex = 0;
    casey = 0;

	while(1)
    {

        for(y = 0;y <8;y++)
        {
            bin[0] = 0;
            bin[1] = 0;
            bin[2] = 0;
            bin[3] = 0;

            for(x = 0;x < 8;x++)
            {
                i = x + (y*image->w) + casex + (casey*image->w);
                l = i*3;
                b = pixel[l+0];
                g = pixel[l+1];
                r = pixel[l+2];

                for(i = 0;i <16;i++)
                {
                    l = i*3;
                    if(b == (palette[l+0]) && g == (palette[l+1]) && r == (palette[l+2]))
                        break;
                }

                //printf("%d %d %d : %d\n",b,v,r,i);

                bin[0] += ( (i>>0) & 0x01 ) << (7 - x);
                bin[1] += ( (i>>1) & 0x01 ) << (7 - x);
                bin[2] += ( (i>>2) & 0x01 ) << (7 - x);
                bin[3] += ( (i>>3) & 0x01 ) << (7 - x);
            }

            for(i = 0;i < 4;i++)
			{
				fputc(bin[i],file);
			}

            size +=4;
        }

        casex += 8;
        if(casex+8 >image->w)
        {
            casex = 0;
            casey += 8;

        }

        if(casey+8 >image->h) break;
    }

    return size;
}


int sms_write_pal(FILE *file,SDL_Surface *image,unsigned char *palette,int ncolor,int mode)
{
    int i,n;
    int psize = 0;
    int color;
    int r,g,b;

    int size = image->w*image->h*image->format->BytesPerPixel;
    unsigned char *pixel = image->pixels;

    if(mode == 3)
    {
        n = 0;
        for(i = 0;i < size;i += image->format->BytesPerPixel)
        {
            palette[n+0] = pixel[i+0];
            palette[n+1] = pixel[i+1];
            palette[n+2] = pixel[i+2];
            n +=3;
            if(n > 768) break;
        }
        ncolor = n/3;
    }

    for(i = 0;i <ncolor;i++)
    {
        n = i*3;

        r = ( (palette[n+0])/85);
        g = ( (palette[n+1])/85);
        b = ( (palette[n+2])/85);

        //printf("%d , %d , %d\n",palette[n+0],palette[n+1],palette[n+2]);

        color = (r<<4) + (g<<2) + b;

        fputc(color&0xFF,file);
        psize++;
    }

    return psize;
}

/*
void sms_convert(SDL_Surface *image,char *adresse,int mode);

int main4(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *image,*copy;
    int n = 1,force = 0,noalpha = 0,i,mode = 0;
    char adresse[1000];
    adresse[0] = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i],"-palette") == 0) mode = 1;


        }else
        {
            strcpy(adresse,argv[i]);
        }
    }

    if(adresse[0] == 0)
    {
        printf("Enter a picture format .png,pcx,bmp, or jpg\n");
        printf("Exemple :\nsmsconvert myimage\n");
        return;
    }

    image = IMG_Load(adresse);
    if(image == NULL)
    {
        printf("Image is not valide\n");
        return;
    }

    copy = SDL_CreateRGBSurface(0,image->w,image->h,24,0,0,0,0);
    SDL_BlitSurface(image,NULL,copy,NULL);

    sms_convert(copy,adresse,mode);

    SDL_FreeSurface(copy);
    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;

}
*/

/*
void sms_palette(SDL_Surface *image,char *adresse,int mode)
{
    FILE *file;
    int i,r,v,b,couleur,l;
    unsigned char *pixel = image->pixels;
    char chaine[1000],schaine[1000];

    i = 0;
    while(adresse[i] != 0 && adresse[i] != '.' )
    {
        schaine[i] = adresse[i];
        i++;
    }
    schaine[i] = 0;

    sprintf(chaine,"%s.asm",schaine);
    file = fopen(chaine,"w");

    if(file == NULL) return;

    sprintf(chaine,"    .db");
    fputs(chaine,file);

    for(i = 0;i <16;i++)
    {
        l = i*3;
        b = ( (pixel[l+0]+5)/85);
        v = ( (pixel[l+1]+5)/85);
        r = ( (pixel[l+2]+5)/85);

        //printf("%d  %d %d\n",pixel[l+0],pixel[l+1],pixel[l+2]);
        couleur = (b<<4) + (v<<2) + r;

        sprintf(chaine," $%.2x",couleur);
        fputs(chaine,file);

        if(i == 15) fputs("\n",file);
        else        fputs(",",file);
    }
    sprintf(chaine,"\n");
    fputs(chaine,file);

    fclose(file);

}

void sms_convert_pal(SDL_Surface *image,char *adresse,int mode);

void sms_convert(SDL_Surface *image,char *adresse,int mode)
{
    if(image->w == 16 && image->h == 1) sms_palette(image,adresse,mode);
    else sms_convert_pal(image,adresse,mode);
}


void sms_convert_pal(SDL_Surface *image,char *adresse,int mode)
{
    FILE *file;
    int i,r,v,b,couleur,l,x,y,t;
    unsigned char *pixel = image->pixels;
    char chaine[1000],schaine[1000],name[1000];
    int octet4[4],size = 0;

    SDL_Surface *copy = IMG_Load("palette.png");
    SDL_Surface *image2 = SDL_CreateRGBSurface(0,copy->w,copy->h,24,0,0,0,0);
    SDL_BlitSurface(copy,NULL,image2,NULL);
    unsigned char *ppixel = image2->pixels;

    i = 0;
    while(adresse[i] != 0 && adresse[i] != '.' )
    {
        schaine[i] = adresse[i];
        i++;
    }
    schaine[i] = 0;

    sprintf(chaine,"%s.asm",schaine);
    file = fopen(chaine,"w");

    if(file == NULL) return;

    int casey = 0,casex = 0;
    while(1)
    {
        sprintf(chaine,"    .db");
        fputs(chaine,file);
        for(y = 0;y <8;y++)
        {
            octet4[0] = 0;
            octet4[1] = 0;
            octet4[2] = 0;
            octet4[3] = 0;

            for(x = 0;x < 8;x++)
            {
                i = x + (y*image->w) + casex + (casey*image->w);
                l = i*3;
                b = ((pixel[l+0]+5)/85);
                v = ((pixel[l+1]+5)/85);
                r = ((pixel[l+2]+5)/85);

                for(i = 0;i <16;i++)
                {
                    l = i*3;
                    if(b == (ppixel[l+0]/85) && v == (ppixel[l+1]/85) && r == (ppixel[l+2]/85))
                        break;
                }

                //printf("%d %d %d : %d\n",b,v,r,i);

                octet4[0] += ( (i>>0) & 0x01 ) << (7 - x);
                octet4[1] += ( (i>>1) & 0x01 ) << (7 - x);
                octet4[2] += ( (i>>2) & 0x01 ) << (7 - x);
                octet4[3] += ( (i>>3) & 0x01 ) << (7 - x);
            }

            sprintf(chaine," $%.2x, $%.2x, $%.2x, $%.2x",octet4[0],octet4[1],octet4[2],octet4[3]);
            fputs(chaine,file);
            size +=4;
            //printf("%s\n",chaine);


            if(y == 7) fputs("\n",file);
            else       fputs(",",file);
        }

        casex += 8;
        if(casex+8 >image->w)
        {
            casex = 0;
            casey += 8;

        }

        if(casey+8 >image->h) break;
        //if(casex+8 >image->w) break;

    }
    fputs("\n",file);

    sprintf(chaine,"\n\n; size $%x",size);
    fputs(chaine,file);

    fclose(file);

    SDL_FreeSurface(image2);
    SDL_FreeSurface(copy);
}*/

