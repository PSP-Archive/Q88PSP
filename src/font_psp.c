// font for Hello World PSP
/*
const unsigned char font[]={
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0x38,0x44,0x82,0x82,0x82,0x44,0x38,0x00,	//??
	0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,	//?~
	0xFE,0x82,0x82,0x82,0x82,0x82,0xFE,0x00,	//??
	0x10,0x28,0x28,0x44,0x44,0x82,0xFE,0x00,	//??
	0x10,0x38,0x7C,0x10,0x10,0x10,0x10,0x00,	//??
	0x00,0x08,0x0C,0xFE,0x0C,0x08,0x00,0x00,	//??
	0x10,0x10,0x10,0x10,0x7C,0x38,0x10,0x00,	//??

	0x00,0x20,0x60,0xFE,0x60,0x20,0x00,0x00,	//??
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x18, 0x18, 0x18,
0x18, 0x00, 0x18, 0x00,
0x00, 0x66, 0x66, 0x66,
0x00, 0x00, 0x00, 0x00,
0x00, 0x66, 0xff, 0x66,
0x66, 0xff, 0x66, 0x00,
0x18, 0x3e, 0x60, 0x3c,
0x06, 0x7c, 0x18, 0x00,
0x00, 0x66, 0x6c, 0x18,
0x30, 0x66, 0x46, 0x00,
0x1c, 0x36, 0x1c, 0x38,
0x6f, 0x66, 0x3b, 0x00,
0x00, 0x18, 0x18, 0x18,
0x00, 0x00, 0x00, 0x00,
0x00, 0x0e, 0x1c, 0x18,
0x18, 0x1c, 0x0e, 0x00,
0x00, 0x70, 0x38, 0x18,
0x18, 0x38, 0x70, 0x00,
0x00, 0x66, 0x3c, 0xff,
0x3c, 0x66, 0x00, 0x00,
0x00, 0x18, 0x18, 0x7e,
0x18, 0x18, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x18, 0x18, 0x30,
0x00, 0x00, 0x00, 0x7e,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x18, 0x18, 0x00,
0x00, 0x06, 0x0c, 0x18,
0x30, 0x60, 0x40, 0x00,
0x00, 0x3c, 0x66, 0x6e,
0x76, 0x66, 0x3c, 0x00,
0x00, 0x18, 0x38, 0x18,
0x18, 0x18, 0x7e, 0x00,
0x00, 0x3c, 0x66, 0x0c,
0x18, 0x30, 0x7e, 0x00,
0x00, 0x7e, 0x0c, 0x18,
0x0c, 0x66, 0x3c, 0x00,
0x00, 0x0c, 0x1c, 0x3c,
0x6c, 0x7e, 0x0c, 0x00,
0x00, 0x7e, 0x60, 0x7c,
0x06, 0x66, 0x3c, 0x00,
0x00, 0x3c, 0x60, 0x7c,
0x66, 0x66, 0x3c, 0x00,
0x00, 0x7e, 0x06, 0x0c,
0x18, 0x30, 0x30, 0x00,
0x00, 0x3c, 0x66, 0x3c,
0x66, 0x66, 0x3c, 0x00,
0x00, 0x3c, 0x66, 0x3e,
0x06, 0x0c, 0x38, 0x00,
0x00, 0x00, 0x18, 0x18,
0x00, 0x18, 0x18, 0x00,
0x00, 0x00, 0x18, 0x18,
0x00, 0x18, 0x18, 0x30,
0x06, 0x0c, 0x18, 0x30,
0x18, 0x0c, 0x06, 0x00,
0x00, 0x00, 0x7e, 0x00,
0x00, 0x7e, 0x00, 0x00,
0x60, 0x30, 0x18, 0x0c,
0x18, 0x30, 0x60, 0x00,
0x00, 0x3c, 0x66, 0x0c,
0x18, 0x00, 0x18, 0x00,
0x00, 0x3c, 0x66, 0x6e,
0x6e, 0x60, 0x3e, 0x00,
0x00, 0x18, 0x3c, 0x66,
0x66, 0x7e, 0x66, 0x00,
0x00, 0x7c, 0x66, 0x7c,
0x66, 0x66, 0x7c, 0x00,
0x00, 0x3c, 0x66, 0x60,
0x60, 0x66, 0x3c, 0x00,
0x00, 0x78, 0x6c, 0x66,
0x66, 0x6c, 0x78, 0x00,
0x00, 0x7e, 0x60, 0x7c,
0x60, 0x60, 0x7e, 0x00,
0x00, 0x7e, 0x60, 0x7c,
0x60, 0x60, 0x60, 0x00,
0x00, 0x3e, 0x60, 0x60,
0x6e, 0x66, 0x3e, 0x00,
0x00, 0x66, 0x66, 0x7e,
0x66, 0x66, 0x66, 0x00,
0x00, 0x7e, 0x18, 0x18,
0x18, 0x18, 0x7e, 0x00,
0x00, 0x06, 0x06, 0x06,
0x06, 0x66, 0x3c, 0x00,
0x00, 0x66, 0x6c, 0x78,
0x78, 0x6c, 0x66, 0x00,
0x00, 0x60, 0x60, 0x60,
0x60, 0x60, 0x7e, 0x00,
0x00, 0x63, 0x77, 0x7f,
0x6b, 0x63, 0x63, 0x00,
0x00, 0x66, 0x76, 0x7e,
0x7e, 0x6e, 0x66, 0x00,
0x00, 0x3c, 0x66, 0x66,
0x66, 0x66, 0x3c, 0x00,
0x00, 0x7c, 0x66, 0x66,
0x7c, 0x60, 0x60, 0x00,
0x00, 0x3c, 0x66, 0x66,
0x66, 0x6c, 0x36, 0x00,
0x00, 0x7c, 0x66, 0x66,
0x7c, 0x6c, 0x66, 0x00,
0x00, 0x3c, 0x60, 0x3c,
0x06, 0x06, 0x3c, 0x00,
0x00, 0x7e, 0x18, 0x18,
0x18, 0x18, 0x18, 0x00,
0x00, 0x66, 0x66, 0x66,
0x66, 0x66, 0x7e, 0x00,
0x00, 0x66, 0x66, 0x66,
0x66, 0x3c, 0x18, 0x00,
0x00, 0x63, 0x63, 0x6b,
0x7f, 0x77, 0x63, 0x00,
0x00, 0x66, 0x66, 0x3c,
0x3c, 0x66, 0x66, 0x00,
0x00, 0x66, 0x66, 0x3c,
0x18, 0x18, 0x18, 0x00,
0x00, 0x7e, 0x0c, 0x18,
0x30, 0x60, 0x7e, 0x00,
0x00, 0x1e, 0x18, 0x18,
0x18, 0x18, 0x1e, 0x00,
0x00, 0x40, 0x60, 0x30,
0x18, 0x0c, 0x06, 0x00,
0x00, 0x78, 0x18, 0x18,
0x18, 0x18, 0x78, 0x00,
0x00, 0x08, 0x1c, 0x36,
0x63, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xff, 0x00,
0x00, 0x18, 0x3c, 0x7e,
0x7e, 0x3c, 0x18, 0x00,
0x00, 0x00, 0x3c, 0x06,
0x3e, 0x66, 0x3e, 0x00,
0x00, 0x60, 0x60, 0x7c,
0x66, 0x66, 0x7c, 0x00,
0x00, 0x00, 0x3c, 0x60,
0x60, 0x60, 0x3c, 0x00,
0x00, 0x06, 0x06, 0x3e,
0x66, 0x66, 0x3e, 0x00,
0x00, 0x00, 0x3c, 0x66,
0x7e, 0x60, 0x3c, 0x00,
0x00, 0x0e, 0x18, 0x3e,
0x18, 0x18, 0x18, 0x00,
0x00, 0x00, 0x3e, 0x66,
0x66, 0x3e, 0x06, 0x7c,
0x00, 0x60, 0x60, 0x7c,
0x66, 0x66, 0x66, 0x00,
0x00, 0x18, 0x00, 0x38,
0x18, 0x18, 0x3c, 0x00,
0x00, 0x06, 0x00, 0x06,
0x06, 0x06, 0x06, 0x3c,
0x00, 0x60, 0x60, 0x6c,
0x78, 0x6c, 0x66, 0x00,
0x00, 0x38, 0x18, 0x18,
0x18, 0x18, 0x3c, 0x00,
0x00, 0x00, 0x66, 0x7f,
0x7f, 0x6b, 0x63, 0x00,
0x00, 0x00, 0x7c, 0x66,
0x66, 0x66, 0x66, 0x00,
0x00, 0x00, 0x3c, 0x66,
0x66, 0x66, 0x3c, 0x00,
0x00, 0x00, 0x7c, 0x66,
0x66, 0x7c, 0x60, 0x60,
0x00, 0x00, 0x3e, 0x66,
0x66, 0x3e, 0x06, 0x06,
0x00, 0x00, 0x7c, 0x66,
0x60, 0x60, 0x60, 0x00,
0x00, 0x00, 0x3e, 0x60,
0x3c, 0x06, 0x7c, 0x00,
0x00, 0x18, 0x7e, 0x18,
0x18, 0x18, 0x0e, 0x00,
0x00, 0x00, 0x66, 0x66,
0x66, 0x66, 0x3e, 0x00,
0x00, 0x00, 0x66, 0x66,
0x66, 0x3c, 0x18, 0x00,
0x00, 0x00, 0x63, 0x6b,
0x7f, 0x3e, 0x36, 0x00,
0x00, 0x00, 0x66, 0x3c,
0x18, 0x3c, 0x66, 0x00,
0x00, 0x00, 0x66, 0x66,
0x66, 0x3e, 0x0c, 0x78,
0x00, 0x00, 0x7e, 0x0c,
0x18, 0x30, 0x7e, 0x00,
0x00, 0x18, 0x3c, 0x7e,
0x7e, 0x18, 0x3c, 0x00,
0x18, 0x18, 0x18, 0x18,
0x18, 0x18, 0x18, 0x18,
0x00, 0x7e, 0x78, 0x7c,
0x6e, 0x66, 0x06, 0x00,
0x08, 0x18, 0x38, 0x78,
0x38, 0x18, 0x08, 0x00,
0x10, 0x18, 0x1c, 0x1e,
0x1c, 0x18, 0x10, 0x00,
0x00, 0x36, 0x7f, 0x7f,
0x3e, 0x1c, 0x08, 0x00,
0x18, 0x18, 0x18, 0x1f,
0x1f, 0x18, 0x18, 0x18,
0x03, 0x03, 0x03, 0x03,
0x03, 0x03, 0x03, 0x03,
0x18, 0x18, 0x18, 0xf8,
0xf8, 0x00, 0x00, 0x00,
0x18, 0x18, 0x18, 0xf8,
0xf8, 0x18, 0x18, 0x18,
0x00, 0x00, 0x00, 0xf8,
0xf8, 0x18, 0x18, 0x18,
0x03, 0x07, 0x0e, 0x1c,
0x38, 0x70, 0xe0, 0xc0,
0xc0, 0xe0, 0x70, 0x38,
0x1c, 0x0e, 0x07, 0x03,
0x01, 0x03, 0x07, 0x0f,
0x1f, 0x3f, 0x7f, 0xff,
0x00, 0x00, 0x00, 0x00,
0x0f, 0x0f, 0x0f, 0x0f,
0x80, 0xc0, 0xe0, 0xf0,
0xf8, 0xfc, 0xfe, 0xff,
0x0f, 0x0f, 0x0f, 0x0f,
0x00, 0x00, 0x00, 0x00,
0xf0, 0xf0, 0xf0, 0xf0,
0x00, 0x00, 0x00, 0x00,
0xff, 0xff, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xff, 0xff,
0x00, 0x00, 0x00, 0x00,
0xf0, 0xf0, 0xf0, 0xf0,
0x00, 0x1c, 0x1c, 0x77,
0x77, 0x08, 0x1c, 0x00,
0x00, 0x00, 0x00, 0x1f,
0x1f, 0x18, 0x18, 0x18,
0x00, 0x00, 0x00, 0xff,
0xff, 0x00, 0x00, 0x00,
0x18, 0x18, 0x18, 0xff,
0xff, 0x18, 0x18, 0x18,
0x00, 0x00, 0x3c, 0x7e,
0x7e, 0x7e, 0x3c, 0x00,
0x00, 0x00, 0x00, 0x00,
0xff, 0xff, 0xff, 0xff,
0xc0, 0xc0, 0xc0, 0xc0,
0xc0, 0xc0, 0xc0, 0xc0,
0x00, 0x00, 0x00, 0xff,
0xff, 0x18, 0x18, 0x18,
0x18, 0x18, 0x18, 0xff,
0xff, 0x00, 0x00, 0x00,
0xf0, 0xf0, 0xf0, 0xf0,
0xf0, 0xf0, 0xf0, 0xf0,
0x18, 0x18, 0x18, 0x1f,
0x1f, 0x00, 0x00, 0x00,
0x78, 0x60, 0x78, 0x60,
0x7e, 0x18, 0x1e, 0x00,
0x00, 0x18, 0x3c, 0x7e,
0x18, 0x18, 0x18, 0x00,
0x00, 0x18, 0x18, 0x18,
0x7e, 0x3c, 0x18, 0x00,
0x00, 0x18, 0x30, 0x7e,
0x30, 0x18, 0x00, 0x00,
0x00, 0x18, 0x0c, 0x7e,
0x0c, 0x18, 0x00, 0x00,
	//7b
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,

	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
	0xCC,0xCC,0x33,0x33,0xCC,0xCC,0x33,0x33,
};
*/

const unsigned char system_font[]={
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x70, 0xc0, 0x70, 0x1b, 0xfb, 0x1f, 0x1b, 0x1b, 
  0x70, 0xc0, 0x70, 0x1b, 0xf6, 0x0c, 0x36, 0x63, 
  0xf0, 0xc0, 0xf0, 0xc6, 0xf4, 0x1c, 0x36, 0x63, 
  0xf0, 0xc0, 0xf0, 0xc0, 0xff, 0x0c, 0x0c, 0x0c, 
  0xf0, 0xc0, 0xf0, 0xcc, 0xf6, 0x32, 0x36, 0x1d, 
  0x20, 0x58, 0xf8, 0xdb, 0xda, 0x1c, 0x1a, 0x1b, 
  0xf0, 0xd8, 0xf0, 0xd8, 0xf4, 0x0c, 0x0c, 0x0f, 
  0xf0, 0xd8, 0xf0, 0xde, 0xf8, 0x1e, 0x06, 0x1e, 
  0xd8, 0xd8, 0xf8, 0xd8, 0xdf, 0x0c, 0x0c, 0x0c, 
  0xc0, 0xc0, 0xc0, 0xcf, 0xfc, 0x0f, 0x0c, 0x0c, 
  0xd8, 0xd8, 0xf8, 0xd8, 0xd9, 0x3b, 0x35, 0x31, 
  0x78, 0xc0, 0xc0, 0x78, 0x18, 0x18, 0x18, 0x1f, 
  0x78, 0xc0, 0xc0, 0x7c, 0x32, 0x3e, 0x36, 0x33, 
  0xf0, 0x80, 0xf0, 0x10, 0xe4, 0x1b, 0x1b, 0x04, 
  0xf0, 0x80, 0xf0, 0x10, 0xef, 0x06, 0x06, 0x0f, 
  0xf0, 0xc8, 0xc8, 0xcf, 0xfc, 0x0f, 0x0c, 0x0f, 
  0xf0, 0xc8, 0xc8, 0xca, 0xf6, 0x06, 0x06, 0x0f, 
  0xf0, 0xc8, 0xc8, 0xce, 0xf3, 0x06, 0x08, 0x1f, 
  0xf0, 0xc8, 0xc8, 0xce, 0xf3, 0x06, 0x03, 0x0e, 
  0xf0, 0xc8, 0xc8, 0xca, 0xf6, 0x1a, 0x3f, 0x06, 
  0x88, 0xc8, 0xa8, 0x99, 0x8a, 0x0c, 0x0a, 0x09, 
  0xf0, 0x80, 0xf0, 0x10, 0xf2, 0x1a, 0x16, 0x12, 
  0xf0, 0xc0, 0xf0, 0xce, 0xf9, 0x1e, 0x19, 0x1e, 
  0x78, 0xc0, 0xc0, 0x78, 0x09, 0x0d, 0x0b, 0x09, 
  0xf8, 0xc0, 0xf0, 0xc0, 0xf9, 0x3b, 0x35, 0x31, 
  0xf0, 0x80, 0xf0, 0x1e, 0xf9, 0x1e, 0x19, 0x1e, 
  0xf0, 0xc0, 0xf0, 0xc0, 0xee, 0x10, 0x10, 0x0e, 
  0x00, 0x08, 0x0c, 0x7e, 0x0c, 0x08, 0x00, 0x00, 
  0x00, 0x10, 0x30, 0x7e, 0x30, 0x10, 0x00, 0x00, 
  0x18, 0x3c, 0x5a, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x00, 0x18, 0x18, 0x18, 0x18, 0x5a, 0x3c, 0x18, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00, 
  0xee, 0x66, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x12, 0x12, 0x7e, 0x24, 0xfe, 0x48, 0x48, 0x00, 
  0x10, 0x7e, 0xd0, 0x7c, 0x16, 0xfc, 0x10, 0x00, 
  0x70, 0x56, 0x7c, 0x18, 0x3e, 0x6a, 0xce, 0x00, 
  0x30, 0x48, 0x48, 0x30, 0xca, 0xcc, 0x72, 0x00, 
  0x38, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00, 
  0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00, 
  0x10, 0xd6, 0x7c, 0x30, 0x7c, 0xd6, 0x10, 0x00, 
  0x00, 0x30, 0x30, 0xfe, 0x30, 0x30, 0x30, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x18, 0x30, 
  0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x10, 0x00, 
  0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x00, 
  0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00, 
  0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x7e, 0x00, 
  0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00, 
  0x3c, 0x66, 0x06, 0x3c, 0x06, 0x66, 0x3c, 0x00, 
  0x0c, 0x1c, 0x3c, 0x6c, 0xfe, 0x0c, 0x0c, 0x00, 
  0xfe, 0xc0, 0xf8, 0x0c, 0x06, 0xcc, 0x78, 0x00, 
  0x3c, 0x60, 0xc0, 0xfc, 0xc6, 0xc6, 0x7c, 0x00, 
  0xfe, 0xc6, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00, 
  0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0x00, 
  0x7c, 0xc6, 0xc6, 0x7e, 0x06, 0x0c, 0x78, 0x00, 
  0x10, 0x38, 0x10, 0x00, 0x10, 0x38, 0x10, 0x00, 
  0x10, 0x38, 0x10, 0x00, 0x00, 0x38, 0x18, 0x30, 
  0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00, 
  0x00, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00, 0x00, 
  0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00, 
  0x7c, 0xc6, 0x06, 0x3c, 0x30, 0x00, 0x30, 0x00, 
  0x7c, 0xc6, 0xbe, 0xb2, 0xbc, 0xc0, 0x7e, 0x00, 
  0x10, 0x38, 0x6c, 0xfe, 0xc6, 0xc6, 0xc6, 0x00, 
  0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00, 
  0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00, 
  0xf8, 0x64, 0x66, 0x66, 0x66, 0x64, 0xf8, 0x00, 
  0xfe, 0xc0, 0xc0, 0xf8, 0xc0, 0xc0, 0xfe, 0x00, 
  0xfe, 0xc0, 0xc0, 0xf8, 0xc0, 0xc0, 0xc0, 0x00, 
  0x3c, 0x62, 0xc0, 0xce, 0xc6, 0x66, 0x3c, 0x00, 
  0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00, 
  0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 
  0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0xcc, 0x78, 0x00, 
  0xc6, 0xcc, 0xd8, 0xf0, 0xd8, 0xcc, 0xc6, 0x00, 
  0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 
  0xc6, 0xee, 0xd6, 0xd6, 0xc6, 0xc6, 0xc6, 0x00, 
  0xc6, 0xc6, 0xe6, 0xd6, 0xce, 0xc6, 0xc6, 0x00, 
  0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 
  0xfc, 0xc6, 0xc6, 0xfc, 0xc0, 0xc0, 0xc0, 0x00, 
  0x38, 0x64, 0xc6, 0xc6, 0xf6, 0xcc, 0x3a, 0x00, 
  0xfc, 0xc6, 0xc6, 0xfc, 0xd8, 0xcc, 0xc6, 0x00, 
  0x7c, 0xc6, 0xc0, 0x3c, 0x06, 0xc6, 0x7c, 0x00, 
  0xfc, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 
  0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 
  0xc6, 0xc6, 0xc6, 0x64, 0x2c, 0x18, 0x18, 0x00, 
  0xc6, 0xc6, 0xc6, 0xd6, 0xd6, 0xee, 0xc6, 0x00, 
  0xc6, 0xc6, 0x28, 0x10, 0x28, 0xc6, 0xc6, 0x00, 
  0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x30, 0x00, 
  0xfe, 0x06, 0x0c, 0x18, 0x60, 0xc0, 0xfe, 0x00, 
  0x7c, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7c, 0x00, 
  0xc6, 0x28, 0x7c, 0x10, 0x7c, 0x10, 0x10, 0x00, 
  0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00, 
  0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x7c, 0x0c, 0x7c, 0xcc, 0x7a, 0x00, 
  0xc0, 0xc0, 0xdc, 0xe6, 0xc6, 0xe6, 0xdc, 0x00, 
  0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc6, 0x7c, 0x00, 
  0x06, 0x06, 0x3e, 0xc6, 0xc6, 0xc6, 0x3a, 0x00, 
  0x00, 0x00, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00, 
  0x1c, 0x36, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x00, 
  0x00, 0x00, 0x7a, 0xc6, 0xc6, 0x7e, 0x06, 0x7c, 
  0xc0, 0xc0, 0xdc, 0xe6, 0xc6, 0xc6, 0xc6, 0x00, 
  0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x1c, 0x00, 
  0x0c, 0x00, 0x1c, 0x0c, 0x0c, 0x0c, 0xcc, 0x38, 
  0xc0, 0xc0, 0xcc, 0xd8, 0xf0, 0xd8, 0xcc, 0x00, 
  0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1c, 0x00, 
  0x00, 0x00, 0xec, 0xd6, 0xd6, 0xd6, 0xd6, 0x00, 
  0x00, 0x00, 0xdc, 0xe6, 0xc6, 0xc6, 0xc6, 0x00, 
  0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 
  0x00, 0x00, 0xfc, 0xc6, 0xc6, 0xfc, 0xc0, 0xc0, 
  0x00, 0x00, 0x7e, 0xc6, 0xc6, 0x7e, 0x06, 0x06, 
  0x00, 0x00, 0xdc, 0xe6, 0xc0, 0xc0, 0xc0, 0x00, 
  0x00, 0x00, 0x7c, 0xc0, 0x7c, 0x06, 0x7c, 0x00, 
  0x30, 0x30, 0xfc, 0x30, 0x30, 0x36, 0x1c, 0x00, 
  0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0x7a, 0x00, 
  0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00, 
  0x00, 0x00, 0xc6, 0xd6, 0xd6, 0xd6, 0x6c, 0x00, 
  0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00, 
  0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0x7c, 
  0x00, 0x00, 0xfe, 0x0c, 0x18, 0x60, 0xfe, 0x00, 
  0x1c, 0x30, 0x30, 0xc0, 0x30, 0x30, 0x1c, 0x00, 
  0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, 0x00, 
  0x70, 0x18, 0x18, 0x06, 0x18, 0x18, 0x70, 0x00, 
  0x60, 0x92, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
  0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 
  0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 
  0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 
  0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 
  0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08, 
  0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08, 0x08, 
  0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0x08, 
  0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08, 
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08, 0x08, 
  0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00, 
  0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x08, 
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x08, 
  0x08, 0x08, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 
  0x08, 0x08, 0x08, 0x10, 0xe0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x10, 0x28, 0x10, 0x00, 
  0x38, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x38, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0x18, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 
  0x7e, 0x06, 0x06, 0x7e, 0x06, 0x0c, 0x78, 0x00, 
  0x00, 0x00, 0xfc, 0x0c, 0x38, 0x30, 0x60, 0x00, 
  0x00, 0x00, 0x0c, 0x18, 0x38, 0x58, 0x18, 0x00, 
  0x00, 0x00, 0x30, 0xfc, 0xcc, 0x0c, 0x38, 0x00, 
  0x00, 0x00, 0x00, 0xfc, 0x30, 0x30, 0xfc, 0x00, 
  0x00, 0x00, 0x18, 0xfc, 0x38, 0x58, 0x98, 0x00, 
  0x00, 0x00, 0x60, 0xfc, 0x6c, 0x68, 0x60, 0x00, 
  0x00, 0x00, 0x00, 0x78, 0x18, 0x18, 0xfc, 0x00, 
  0x00, 0x00, 0x7c, 0x0c, 0x7c, 0x0c, 0x7c, 0x00, 
  0x00, 0x00, 0x00, 0xa4, 0x54, 0x04, 0x18, 0x00, 
  0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
  0xfe, 0x06, 0x26, 0x34, 0x38, 0x30, 0x60, 0x00, 
  0x06, 0x0c, 0x18, 0x38, 0x78, 0xd8, 0x18, 0x00, 
  0x18, 0xfe, 0xc6, 0xc6, 0x06, 0x0c, 0x38, 0x00, 
  0x00, 0x7c, 0x30, 0x30, 0x30, 0x30, 0xfe, 0x00, 
  0x0c, 0xfe, 0x0c, 0x3c, 0x6c, 0xcc, 0x0c, 0x00, 
  0x30, 0xfe, 0x36, 0x36, 0x36, 0x66, 0xcc, 0x00, 
  0x30, 0xfe, 0x30, 0xfe, 0x18, 0x18, 0x18, 0x00, 
  0x3e, 0x66, 0xc6, 0x0c, 0x18, 0x30, 0x60, 0x00, 
  0x60, 0x7e, 0x6c, 0x8c, 0x08, 0x18, 0x30, 0x00, 
  0x00, 0x7e, 0x06, 0x06, 0x06, 0x06, 0x7e, 0x00, 
  0x6c, 0xfe, 0x6c, 0x0c, 0x0c, 0x18, 0x70, 0x00, 
  0x00, 0x70, 0x00, 0x72, 0x06, 0x4c, 0x78, 0x00, 
  0xfe, 0x06, 0x0c, 0x18, 0x38, 0x6c, 0xc6, 0x00, 
  0x60, 0xfe, 0x66, 0x6c, 0x60, 0x60, 0x3e, 0x00, 
  0xc6, 0x66, 0x26, 0x06, 0x04, 0x18, 0x70, 0x00, 
  0x3e, 0x66, 0xe6, 0x3c, 0x0c, 0x10, 0x60, 0x00, 
  0x0c, 0x38, 0x18, 0xfe, 0x18, 0x18, 0x70, 0x00, 
  0x00, 0xda, 0xda, 0xda, 0x02, 0x0c, 0x70, 0x00, 
  0x7c, 0x00, 0xfe, 0x18, 0x18, 0x30, 0x60, 0x00, 
  0x30, 0x30, 0x30, 0x3c, 0x36, 0x30, 0x30, 0x00, 
  0x18, 0x18, 0xfe, 0x18, 0x18, 0x30, 0x60, 0x00, 
  0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 
  0x00, 0x7e, 0x06, 0x24, 0x18, 0x2c, 0x66, 0x00, 
  0x18, 0x7e, 0x0c, 0x18, 0x3c, 0xda, 0x18, 0x00, 
  0x06, 0x06, 0x06, 0x06, 0x0c, 0x10, 0x20, 0x00, 
  0x30, 0x18, 0x0c, 0x66, 0x66, 0xc6, 0xc6, 0x00, 
  0xc0, 0xc0, 0xfe, 0xc0, 0xc0, 0xc0, 0x7e, 0x00, 
  0x00, 0xfe, 0x06, 0x06, 0x0c, 0x18, 0x70, 0x00, 
  0x00, 0x10, 0x68, 0xcc, 0x06, 0x02, 0x02, 0x00, 
  0x18, 0x18, 0x7e, 0x18, 0x5c, 0x9a, 0x18, 0x00, 
  0x7e, 0x06, 0x06, 0x0c, 0x68, 0x30, 0x18, 0x00, 
  0x00, 0x38, 0x00, 0x38, 0x00, 0x78, 0x04, 0x00, 
  0x04, 0x18, 0x30, 0x60, 0xd8, 0xfc, 0x06, 0x00, 
  0x00, 0x06, 0x06, 0x6c, 0x18, 0x34, 0xe2, 0x00, 
  0x00, 0xfe, 0x30, 0xfe, 0x30, 0x38, 0x1e, 0x00, 
  0x30, 0x30, 0xfe, 0x16, 0x1c, 0x18, 0x18, 0x00, 
  0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0xfe, 0x00, 
  0x7e, 0x06, 0x06, 0x7e, 0x06, 0x06, 0x7e, 0x00, 
  0x7c, 0x00, 0xfe, 0x06, 0x0c, 0x18, 0x30, 0x00, 
  0xc6, 0xc6, 0xc6, 0x06, 0x06, 0x0c, 0x78, 0x00, 
  0x68, 0x68, 0x68, 0x6a, 0x6e, 0x6c, 0xc8, 0x00, 
  0x60, 0x60, 0x60, 0x66, 0x6c, 0x78, 0x70, 0x00, 
  0x00, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0x00, 
  0x00, 0xfe, 0xc6, 0xc6, 0x06, 0x0c, 0x38, 0x00, 
  0x00, 0x70, 0x72, 0x06, 0x06, 0x6c, 0x70, 0x00, 
  0x18, 0xcc, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x70, 0xd8, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
  0x08, 0x08, 0x0f, 0x08, 0x08, 0x0f, 0x08, 0x08, 
  0x08, 0x08, 0xff, 0x08, 0x08, 0xff, 0x08, 0x08, 
  0x08, 0x08, 0xf8, 0x08, 0x08, 0xf8, 0x08, 0x08, 
  0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 
  0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 
  0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 
  0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 
  0x10, 0x38, 0x7c, 0xfe, 0xfe, 0x10, 0x38, 0x00, 
  0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x10, 0x00, 
  0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00, 
  0x10, 0x38, 0x54, 0xfe, 0x56, 0x10, 0x38, 0x00, 
  0x00, 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c, 0x00, 
  0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 
  0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81, 
  0xff, 0xdb, 0xdb, 0xff, 0xc3, 0xc3, 0xc3, 0x00, 
  0x60, 0x7e, 0xd8, 0x3e, 0x68, 0xfe, 0x18, 0x00, 
  0x7e, 0x66, 0x7e, 0x66, 0x7e, 0x66, 0xc6, 0x00, 
  0xfe, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xfe, 0x00, 
  0x04, 0xee, 0xa4, 0xfe, 0xac, 0xfe, 0x14, 0x04, 
  0x00, 0x38, 0x6c, 0xc6, 0x3c, 0x34, 0x6c, 0x00, 
  0x74, 0x64, 0xfe, 0x6e, 0x76, 0xe4, 0x6c, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 
  0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 
  0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 
  0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 
  0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 
  0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 
  0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
  0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 
  0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 
  0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 
  0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 
  0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 
  0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 
  0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 
  0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 
  0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
  0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 
  0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 
  0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 
  0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 
  0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 
  0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 
  0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 
  0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 
  0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 
  0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 
  0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 
  0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 
  0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 
  0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 
  0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 
  0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 
  0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 
  0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 
  0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 
  0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 
  0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 
  0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 
  0xff, 0xff, 0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 
  0x0f, 0x0f, 0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 
  0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 
  0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 
  0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 
  0x00, 0x00, 0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 
  0x00, 0x00, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 
  0xf0, 0xf0, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 
  0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 
  0xff, 0xff, 0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 
  0x0f, 0x0f, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 
  0xff, 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 
  0x0f, 0x0f, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 
  0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 
  0xf0, 0xf0, 0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 
  0x00, 0x00, 0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 
  0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xf0, 0xf0, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 
  0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 
  0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 
  0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 
  0x0f, 0x0f, 0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 
  0xff, 0xff, 0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 
  0x0f, 0x0f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 
  0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 
  0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 
  0xf0, 0xf0, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 
  0x00, 0x00, 0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 
  0xf0, 0xf0, 0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 
  0x00, 0x00, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 
  0xf0, 0xf0, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 
  0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 
  0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 
  0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 
  0xff, 0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 
  0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 
  0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 
  0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
};
