/*
 * SAM_BA_loader.c
 *
 * Created: 4.5.2014 19:40:00
 *  Author: kubas
 */ 

#include "sam.h"

#define source_address		(*(      uint32_t* const) (0x2007C800))
#define destination_address	(*(      uint32_t* const) (0x2007C804))
#define start_page			(*(      uint32_t* const) (0x2007C808))
#define pages_count			(*(const uint32_t* const) (0x2007C80C))
#define page_size			(*(const uint32_t* const) (0x2007C810))
#define cmd					(*(      uint32_t* const) (0x2007C814))

int main(void)
{
	switch(cmd)
	{
	case 1: // write pages
		for(uint32_t page = 0; page != pages_count; ++page)
		{
			for(uint32_t page_offset = 0; page_offset != page_size; page_offset += 4)
			{
				*((uint32_t*)(destination_address)) = *((uint32_t*)(source_address + page_offset));
				destination_address += 4;
			}
			EFC0->EEFC_FCR = EEFC_FCR_FCMD_EWP | EEFC_FCR_FARG(start_page) | EEFC_FCR_FKEY_PASSWD;
			while((EFC0->EEFC_FSR & EEFC_FSR_FRDY) == 0) {};
			++start_page;
		}
		//cmd = 0;
		break;
	case 2: //set booting from flash and reset
		while(!(EFC0->EEFC_FSR & EEFC_FSR_FRDY)) {}
		EFC0->EEFC_FCR = EEFC_FCR_FCMD_SGPB | EEFC_FCR_FARG(1) | EEFC_FCR_FKEY_PASSWD;
		while(!(EFC0->EEFC_FSR & EEFC_FSR_FRDY)) {}
		while(RSTC->RSTC_SR & RSTC_SR_SRCMP) {}
		RSTC->RSTC_CR = RSTC_CR_PROCRST | RSTC_CR_PERRST | RSTC_CR_KEY(0xA5);
		for(;;) {}
	case 3: //set booting from rom and reset
		while(!(EFC0->EEFC_FSR & EEFC_FSR_FRDY)) {}
		EFC0->EEFC_FCR = EEFC_FCR_FCMD_CGPB | EEFC_FCR_FARG(1) | EEFC_FCR_FKEY_PASSWD;
		while(!(EFC0->EEFC_FSR & EEFC_FSR_FRDY)) {}
		while(RSTC->RSTC_SR & RSTC_SR_SRCMP) {}
		RSTC->RSTC_CR = RSTC_CR_PROCRST | RSTC_CR_PERRST | RSTC_CR_KEY(0xA5);
		for(;;) {}
	}
}
