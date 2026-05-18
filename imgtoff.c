#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char gradient[] = " .:-=+*#%@";

void resize(unsigned char *image, unsigned char *new_image,
		int orig_w, int orig_h, int new_w, int new_h)
{
	double ratio_w, ratio_h, x0, y0, ofst_w, ofst_h;
	int x1, x2, y1, y2; 
	unsigned char p1_1, p1_2, p2_1, p2_2, t1, t2, t3, v;
	ratio_w = (double) orig_w / new_w;
	ratio_h = (double) orig_h / new_h;
	ofst_w = (orig_w > new_w) ? (1 - 1.0/ratio_w) : 0;
	ofst_h = (orig_h > new_h) ? (1 - 1.0/ratio_h) : 0;

	for (int y = 0; y < new_h; y++) {
		for (int x = 0; x < new_w; x++) {
			x0 = x * ratio_w + ofst_w;
			y0 = y * ratio_h + ofst_h;
			x1 = floor(x0);
			y1 = floor(y0);
			x2 = ceil(x0);
			y2 = ceil(y0);
			x0 -= x1;
			y0 -= y1;
			for (int c = 0; c < 3; c++) {
				p1_1 = image[(y1 * orig_w + x1) * 3 + c];
				p1_2 = image[(y2 * orig_w + x1) * 3 + c];
				p2_1 = image[(y1 * orig_w + x2) * 3 + c];
				p2_2 = image[(y2 * orig_w + x2) * 3 + c];
				t1 = p1_1 * (1 - x0) + p2_1 * x0;
				t2 = p1_2 * (1 - x0) + p2_2 * x0;
				t3 = t1 * (1 - y0) + t2 * y0;
				new_image[(y * new_w + x) * 3 + c] = t3;
			}
		}
	}
}

void iterative_downscale(unsigned char *orig_image, unsigned char **new_image,
		int orig_w, int orig_h, int new_w, int new_h)
{
	unsigned char *temp_image;
	float ratio_w, ratio_h;
	for (;;) {
		ratio_w = (float) orig_w / new_w; 
		ratio_h = (float) orig_h / new_h; 
		if (ratio_w > 2 && ratio_h > 2) {
			temp_image = (unsigned char *)
					malloc((orig_w / 2) * (orig_h / 2) * 3);
			resize(orig_image, temp_image, orig_w, orig_h,
					orig_w / 2, orig_h / 2);
			orig_w /= 2;
			orig_h /= 2;
			free(orig_image);
			orig_image = temp_image;
		} else if (ratio_w > 2) {
			temp_image = (unsigned char *)
					malloc((orig_w / 2) * (orig_h) * 3);
			resize(orig_image, temp_image, orig_w, orig_h,
					orig_w / 2, orig_h);
			orig_w /= 2;
			free(orig_image);
			orig_image = temp_image;
		} else if (ratio_h > 2) {
			temp_image = (unsigned char *)
					malloc((orig_w) * (orig_h / 2) * 3);
			resize(orig_image, temp_image, orig_w, orig_h,
					orig_w, orig_h / 2);
			orig_h /= 2;
			free(orig_image);
			orig_image = temp_image;
		} else {
			temp_image = (unsigned char *)
					malloc(new_w * new_h * 3);
			resize(orig_image, temp_image, orig_w, orig_h,
					new_w, new_h);
			free(orig_image);
			(*new_image) = temp_image;
			return;
		}
	}
}

unsigned char *get_values_image(unsigned char *image, int w, int h)
{	
	unsigned char *value_image, v;
	value_image = (unsigned char *) malloc(w * h);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			v = image[(y * w + x) * 3 + 0];
			for (int c = 1; c < 3; c++) {
				if (image[(y * w + x) * 3 + c] > v)
					v = image[(y * w + x) * 3 + c];
			}
			value_image[y * w + x] = gradient[v * 10 / 256]; 
		}
	}
	return value_image;
}

unsigned char get_color_index(short r, short g, short b)
{
	float r_, g_, b_, cmax, cmin, delta, hue, sat;
	r_ = r / 255.0;
	g_ = g / 255.0;
	b_ = b / 255.0;
	cmax = fmax(r_, fmax(g_, b_));
	cmin = fmin(r_, fmin(g_, b_));
	delta = cmax - cmin;
	if (delta == 0)
		hue = 0;
	else if (cmax == r_)
		hue = fmod((g_ - b_) / delta, 6);
	else if (cmax == g_)
		hue = ((b_ - r_) / delta + 2);
	else if (cmax == b_)
		hue = ((r_ - g_) / delta + 4);
	if (hue < 0)
		hue += 6;
	sat = (cmax == 0) ? 0 : delta / cmax;
	if (sat < 0.2)
		return 7;
	if (hue >= 0.5 && hue < 1.5)
		return 3;
	if (hue >= 1.5 && hue < 2.5)
		return 2;
	if (hue >= 2.5 && hue < 3.5)
		return 6;
	if (hue >= 3.5 && hue < 4.5)
		return 4;
	if (hue >= 4.5 && hue < 5.5)
		return 5;
	return 1;
}

unsigned char *get_colors_image(unsigned char *image, int w, int h)
{	
	unsigned char *color_image;
	color_image = (unsigned char *) malloc(w * h);
	unsigned char c;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			c = get_color_index(
					image[(y * w + x) * 3 + 0],
					image[(y * w + x) * 3 + 1],
					image[(y * w + x) * 3 + 2]);
			color_image[y * w + x] = c; 
		}
	}
	return color_image;
}

void write_final_image(unsigned char *value_image, unsigned char *color_image,
		int w, int h, char *path)
{
	FILE *txt;
	unsigned char colors[7];
	int colors_size, cc, nc, i;
	txt = fopen(path, "w");

	colors_size = 0;
	cc = 0;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			nc = color_image[y * w + x];
			if (nc != cc && nc != 0) {
				i = 0;
				while (i < colors_size && nc != colors[i])
					++i;
				if (i == colors_size)
					colors[colors_size++] = nc;
				if (cc != 0)
					fprintf(txt, "$%d", i + 1);
				cc = nc;
			}
			fprintf(txt, "%c", value_image[y * w + x]);
		}
		fprintf(txt, "\n");
	}
	fclose(txt);
	free(value_image);
	free(color_image);

	printf("fastfetch --logo \"%s\" ", path);
	for (i = 0; i < colors_size; i++) {
		printf("--logo-color-%d \"3%d\" ", i + 1, colors[i]);
	}
	printf("\n");
}

unsigned char *convert_rgba_to_rgb(unsigned char *orig_image, int w, int h)
{
	unsigned char *new_image;
	new_image = (unsigned char *) malloc(w * h * 3);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			new_image[(y * w + x) * 3 + 0] =
					orig_image[(y * w + x) * 4 + 0] * 
					orig_image[(y * w + x) * 4 + 3] / 255;
			new_image[(y * w + x) * 3 + 1] =
					orig_image[(y * w + x) * 4 + 1] * 
					orig_image[(y * w + x) * 4 + 3] / 255;
			new_image[(y * w + x) * 3 + 2] =
					orig_image[(y * w + x) * 4 + 2] * 
					orig_image[(y * w + x) * 4 + 3] / 255;
		}
	}
	free(orig_image);
	return new_image;
}

int main(int argc, char **argv)
{
	int orig_w, orig_h, new_w, new_h, c_num, slen;
	unsigned char *orig_image, *new_image, *temp_image;
	unsigned char *value_image, *color_image;

	if (argc < 5) {
		printf("Insufficient arguments.\nUSAGE: %s {INPUT} {WIDTH} {HEIGHT} {OUTPUT}");
		return 1;
	}

	orig_image = stbi_load(argv[1], &orig_w, &orig_h, &c_num, 0);
	if (c_num == 4)
		orig_image = convert_rgba_to_rgb(orig_image, orig_w, orig_h);
	new_w = atoi(argv[2]);
	new_h = atoi(argv[3]);

	iterative_downscale(orig_image, &new_image,
			orig_w, orig_h, new_w, new_h);

	value_image = get_values_image(new_image, new_w, new_h);
	color_image = get_colors_image(new_image, new_w, new_h);
	free(new_image);
	new_image = NULL;

	write_final_image(value_image, color_image, new_w, new_h, argv[4]);
	value_image = NULL;
	color_image = NULL;
	return 0;
}
