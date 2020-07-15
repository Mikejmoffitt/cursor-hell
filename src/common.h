#ifndef COMMON_H
#define COMMON_H

#define SAFELOAD_BMP(_symbol_, _fname_) \
	{\
		SDL_Surface *surf = IMG_Load(_fname_); \
		if (!surf)\
		{\
			printf("[safeload]: %s\n", SDL_GetError()); \
			_symbol_ = SDL_CreateTexture(disp_get_renderer(), \
			                             SDL_PIXELFORMAT_RGBA8888, \
			                             SDL_TEXTUREACCESS_STATIC, \
			                             16, 16); \
		}\
		else\
		{\
			_symbol_ = SDL_CreateTextureFromSurface(disp_get_renderer(), surf); \
			if (!_symbol_) \
			{\
				printf("[safeload]: %s\n", SDL_GetError()); \
				_symbol_ = SDL_CreateTexture(disp_get_renderer(), \
				                             SDL_PIXELFORMAT_RGBA8888, \
					               	         SDL_TEXTUREACCESS_STATIC, \
					                         16, 16); \
			}\
			else\
			{\
				SDL_FreeSurface(surf); \
			}\
		}\
	} \

// Macro to get the number of indeces in an array.
#define ARRAYSIZE(_symbol_) (sizeof(_symbol_) / sizeof(_symbol_[0]))

#endif  // COMMON_H
