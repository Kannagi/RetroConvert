#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "retro.h"

int nes_write_pal(FILE *file,SDL_Surface *image,unsigned char *palette,int ncolor,int mode)
{
    int i,n;
    int psize = 0;
    unsigned char color,color2;
    int r,g,b,chrome;

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
            if(n >= 0x300) break;
        }
        ncolor = n/3;
    }

    for(i = 0;i < ncolor;i++)
	{
		n = i*3;

		r = palette[n+2]/63;
		g = palette[n+1]/63;
		b = palette[n+0]/63;

		color = 0x0F;
		color2 = 1;
		chrome = 0;

		//printf("%d %d %d :",r,g,b);

		if(b >= 2)
			chrome |= 1;

		if(r >= 2)
			chrome |= 2;

		if(g >= 2)
			chrome |= 4;

		//printf("%d %d %d\n",r,g,b);
		if( (r == g) &&  (g == b) )
		{
			chrome = 0;
			if(r == 1)
			{
				color = 0x0D;
				color2 = 2;
			}

			if(r == 2)
			{
				color = 0x00;
				color2 = 0;
			}

			if(r == 3)
			{
				color = 0x00;
				color2 = 1;
			}

			if(r == 4)
			{
				color = 0x00;
				color2 = 2;
			}
		}

		int Bcolor = b-2;
		int Rcolor = r-2;
		int Gcolor = g-2;

		//blue
		if(chrome == 1)
		{
			color = 0x01;

			if(r > g)
				color = 0x02;

			if( (r > 0) || (g > 0) )
				color2 = 1+Bcolor;
		}

		//magenta
		if(chrome == 3)
		{
			color = 0x03;
			if(g > 0)
				color2 = 1+Bcolor;

			if(r >= b)
			{
				color = 0x04;
				if(g > 0)
					color2 = 1+Rcolor;
			}
		}

		//red
		if(chrome == 2)
		{
			color = 0x05;

			if(g > b)
				color = 0x06;

			if( (b > 0) || (g > 0) )
				color2 = 1+Rcolor;
		}

		//yellow
		if(chrome == 6)
		{
			color = 0x07;
			if(b > 0)
				color2 = 1+Rcolor;

			if(g >= r)
			{
				color = 0x08;
				if(b > 0)
					color2 = 1+Gcolor;
			}
		}

		//green
		if(chrome == 4)
		{
			color = 0x09;

			if(b > r)
				color = 0x0A;

			if( (b > 0) || (r > 0) )
				color2 = 1+Gcolor;
		}

		//cyan
		if(chrome == 5)
		{
			color = 0x0B;
			if(r > 0)
				color2 = 1+Rcolor;

			if(b >= g)
			{
				color = 0x0C;
				if(r > 0)
					color2 = 1+Gcolor;
			}
		}



		color = color + (color2<<4);
		//printf("%x\n",color);
		fputc(color,file);

		psize ++;
	}

    return psize;
}

int nes_write_rom(FILE *file,SDL_Surface *image,unsigned char *palette,int npal,int type)
{
    int blocx,blocy;
    int tiles[64];
    int nespixel[8];
    int i;
    int x,y,size = 0;
    unsigned char bin[16];
	unsigned char *pixel = image->pixels;
    blocx = 0;
    blocy = 0;

    while(1)
    {
        bloc_palette(image,blocx,blocy,8,pixel,palette,tiles);



		for(y = 0;y <8;y++)
		{
			for(i = 0;i < 8;i++)
				nespixel[i] = 0;

			for(x = 0;x < 8;x++)
			{
				i = tiles[x + (y*8)];

				if(i > 3) i = 3;

				nespixel[0] += ( (i&0x01)>>0 ) << (7 - x);
				nespixel[1] += ( (i&0x02)>>1 ) << (7 - x);
			}

			bin[y+0] = nespixel[0] ;
			bin[y+8] = nespixel[1] ;

		}

		fwrite(bin,1 ,16,file);


		size += 16;
        blocx += 8;
        if(blocx+8 >image->w)
        {
            blocx = 0;
            blocy += 8;
        }

        if(blocy+8 >image->h) break;
    }


    return size;
}

/*
{
    int i,n;
    int psize = 0;
    unsigned char color,color2;
    int r,g,b,chrome;

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
            if(n >= 0x300) break;
        }
        ncolor = n/3;
    }


    for(i = 0;i < ncolor;i++)
    {
        n = i*3;

        r = palette[n+0];
        g = palette[n+1];
        b = palette[n+2];
        color = 0x0F;
        color2 = 0;

        chrome = 0;
		if(b >= 128)
			chrome |= 1;

		if(r >= 128)
			chrome |= 2;

		if(g >= 128)
			chrome |= 4;

		int Bcolor = (b-129)/42;
		int Rcolor = (r-129)/42;
		int Gcolor = (g-129)/42;
		if(Bcolor < 0) Bcolor = 0;
		if(Rcolor < 0) Rcolor = 0;
		if(Gcolor < 0) Gcolor = 0;

		//blue
		if(chrome == 1)
		{
			color = 1;
			color2 = Bcolor;

			if(r >= 64)
				color = 2;
		}

		//magenta
		if(chrome == 3)
		{
			color = 3;
			color2 = Bcolor;

			if(r > b)
			{
				color = 4;
				color2 = Rcolor;
			}

		}

		//red
		if(chrome == 2)
		{
			color = 5;
			color2 = Rcolor;

			if(g >= 64)
				color = 6;
		}

		//yellow
		if(chrome == 6)
		{
			color = 7;
			color2 = Rcolor;

			if(g > r)
			{
				color = 8;
				color2 = Gcolor;
			}
		}

		//green
		if(chrome == 4)
		{
			color = 9;
			color2 = Gcolor;

			if(b >= 64)
				color = 10;
		}

		//cyan
		if(chrome == 5)
		{
			color = 11;
			color2 = Gcolor;

			if(b > g)
			{
				color = 12;
				color2 = Bcolor;
			}
		}

		if(chrome == 7)
		{
			color = 0x00;
			color2 = (Rcolor+Gcolor+Bcolor)/3;
		}

		if(color == 0x0F)
		{
			color = (r+g+b)/3;

			if(color >= 30)
				color2 = 2;

			if(color >= 80)
				color2 = 3;

			color = 0xF;
		}

		color = color + (color2<<4);
		fputc(color,file);

        psize += 2;
    }

    return psize;
}*/
