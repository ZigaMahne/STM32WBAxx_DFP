/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 - 2022 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        24. JANUARY 2020
 * $Revision:    V0.0.1
 *
 * Project:      Flash Programming Functions for ST STM32WBAx Flash
 * --------------------------------------------------------------------------- */

/* History: *  
 *  Version 0.0.1
 *    Initial release
 */

/* Note:
   Flash has 8K sector size.
   STM32WBAx devices have 2MB Flash SIZE .  */

#include "..\FlashOS.h"        // FlashOS Structures


typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;
typedef volatile unsigned long    vu64;
typedef          unsigned long     u64;



#define M32(adr) (*((vu32 *) (adr)))
#define M64(adr) (*((vu64 *) (adr)))

// Peripheral Memory Map
#define FLASH_BASE        (0x40022000)
#define DBGMCU_BASE       (0xE0044000)

#define FLASH           ((FLASH_TypeDef  *) FLASH_BASE)
#define DBGMCU          ((DBGMCU_TypeDef *) DBGMCU_BASE)
#define FLASHSIZE_BASE    (0x0BFA07A0)



// Debug MCU
typedef struct {
  vu32 IDCODE;
} DBGMCU_TypeDef;

// Flash Registers
typedef struct
{
  vu32 ACR;              /*!< FLASH access control register,                  Address offset: 0x00 */
       vu32 RESERVED0;        /*!< RESERVED1,                                      Address offset: 0x04 */
  vu32 NSKEYR;           /*!< FLASH non-secure key register,                  Address offset: 0x08 */
  vu32 SECKEYR;          /*!< FLASH secure key register,                      Address offset: 0x0C */
  vu32 OPTKEYR;          /*!< FLASH option key register,                      Address offset: 0x10 */
       vu32 RESERVED1;        /*!< Reserved1,                                      Address offset: 0x14 */
  vu32 PDKEYR;           /*!< FLASH Bank power-down key register,             Address offset: 0x18 */
       vu32 RESERVED2;        /*!< Reserved2,                                      Address offset: 0x1C */
  vu32 NSSR;             /*!< FLASH non-secure status register,               Address offset: 0x20 */
  vu32 SECSR;            /*!< FLASH secure status register,                   Address offset: 0x24 */
  vu32 NSCR1;            /*!< FLASH non-secure control register,              Address offset: 0x28 */
  vu32 SECCR1;           /*!< FLASH secure control register,                  Address offset: 0x2C */
  vu32 ECCR;             /*!< FLASH ECC register,                             Address offset: 0x30 */
  vu32 OPSR;             /*!< FLASH OPSR register,                            Address offset: 0x34 */
  vu32 NSCR2;            /*!< FLASH non-secure control register,              Address offset: 0x38 */
  vu32 SECCR2;           /*!< FLASH secure control register,                  Address offset: 0x3C */
  vu32 OPTR;             /*!< FLASH option control register,                  Address offset: 0x40 */
  vu32 NSBOOTADD0R;      /*!< FLASH non-secure boot address 0 register,       Address offset: 0x44 */
  vu32 NSBOOTADD1R;      /*!< FLASH non-secure boot address 1 register,       Address offset: 0x48 */
  vu32 SECBOOTADD0R;     /*!< FLASH secure boot address 0 register,           Address offset: 0x4C */
  vu32 SECWMR1 ;         /*!< FLASH secure watermark1 register 1,             Address offset: 0x50 */
  vu32 SECWMR2;          /*!< FLASH secure watermark1 register 2,             Address offset: 0x54 */
  vu32 WRPAR;            /*!< FLASH WRP area A address register,              Address offset: 0x58 */
  vu32 WRPBR;            /*!< FLASH WRP area B address register,              Address offset: 0x5C */
  vu32 RESERVED3[4];     /*!< Reserved3,                                      Address offset: 0x60-0x6C */
  vu32 OEM1KEYR1;        /*!< FLASH OEM1 key register 1,                      Address offset: 0x70 */
  vu32 OEM1KEYR2;        /*!< FLASH OEM1 key register 2,                      Address offset: 0x74 */
  vu32 OEM2KEYR1;        /*!< FLASH OEM2 key register 1,                      Address offset: 0x78 */
  vu32 OEM2KEYR2;        /*!< FLASH OEM2 key register 2,                      Address offset: 0x7C */
  vu32 SECBBR1;          /*!< FLASH secure block-based bank register 1,       Address offset: 0x80 */
  vu32 SECBBR2;          /*!< FLASH secure block-based bank register 2,       Address offset: 0x84 */
  vu32 SECBBR3;          /*!< FLASH secure block-based bank register 3,       Address offset: 0x88 */
  vu32 SECBBR4;          /*!< FLASH secure block-based bank register 4,       Address offset: 0x8C */
  vu32 RESERVED4[4];    /*!< Reserved4,                                      Address offset: 0x90-0x9C */
  vu32 SECBB2R1;         /* offset: 0x1A0 FLASH secure block-based bank 2 register 1 */
  vu32 SECBB2R2;         /* offset: 0x1A4 FLASH secure block-based bank 2 register 2 */
  vu32 SECBB2R3;         /* offset: 0x1A8 FLASH secure block-based bank 2 register 3 */
  vu32 SECBB2R4;         /* offset: 0x1AC FLASH secure block-based bank 2 register 4 */
} FLASH_TypeDef;

// Flash Keys
#define FLASH_KEY1               0x45670123
#define FLASH_KEY2               0xCDEF89AB
#define FLASH_OPTKEY1            0x08192A3B
#define FLASH_OPTKEY2            0x4C5D6E7F

// Flash Control Register definition
#define FLASH_PG                ((u32)(1U <<  0))
#define FLASH_PER               ((u32)(1U <<  1))
#define FLASH_MER1              ((u32)(1U <<  2))
#define FLASH_MER2              ((u32)(1U << 15))
#define FLASH_PNB_MSK           ((u32)(0x7F << 3))
#define FLASH_BKER              ((u32)(1U << 11))
#define FLASH_STRT              ((u32)(1U << 16))
#define FLASH_LOCK              ((u32)(1U << 31))


// Flash Status Register definition
#define FLASH_EOP               ((u32)(1U <<  0))
#define FLASH_OPERR             ((u32)(1U <<  1))
#define FLASH_PROGERR           ((u32)(1U <<  3))
#define FLASH_WRPERR            ((u32)(1U <<  4))
#define FLASH_PGAERR            ((u32)(1U <<  5))
#define FLASH_SIZERR            ((u32)(1U <<  6))
#define FLASH_PGSERR            ((u32)(1U <<  7))
#define FLASH_OPTWERR           ((u32)(1U <<  13))
#define FLASH_BSY               ((u32)(1U << 16))
#define FLASH_WDW               ((u32)(1U << 17))

 
#define FLASH_PGERR             (  FLASH_EOP| FLASH_OPERR  |FLASH_PROGERR| FLASH_WRPERR| FLASH_PGAERR | FLASH_SIZERR  | \
                                 FLASH_PGSERR | FLASH_OPTWERR )

// Flash option register definitions
#define FLASH_OPTR_RDP          ((u32)(0xFF ))
#define FLASH_OPTR_RDP_55       ((u32)(0x55  ))
#define FLASH_OBL_LAUNCH        ((u32)( 1U << 27))
#define FLASH_OPTR_TZEN         ((u32)( 1U << 31))



static void DSB(void) {
    __asm("DSB");
}

/*
 * Get Flash security Mode
 *    Return Value:   0 = non-secure Flash
 *                    1 = secure Flash
 */

static u32 GetFlashSecureMode (void) {
  u32 flashSecureMode;

  flashSecureMode = (FLASH->OPTR & FLASH_OPTR_TZEN) ? 1U : 0U;

  return (flashSecureMode);
}

static u32 GetFlashBank (void) {
  u32 flashSize;

  flashSize = (*(u32*) FLASHSIZE_BASE);
	
	flashSize=((((flashSize & 0xFFFF)*0x400))/2)+0x8000000;
	
  return (flashSize);
}

static u32 GetFlashSize (void) {
  u32 flashSize;

  flashSize = (*(u32*) FLASHSIZE_BASE);
	
	flashSize=(((flashSize & 0xFFFF)*0x400));
	
  return (flashSize);
}



/*
 * Get Flash Type
 *    Return Value:   0 = Single-Bank flash
 *                    1 = Dual-Bank Flash (configurable)
 */

#if defined FLASH_MEM
static u32 GetFlashType (void) {
  u32 flashType = 0U;
	u32 DBGMCU_val; 
   DBGMCU_val= DBGMCU->IDCODE & 0xFFFU;

	if( DBGMCU_val==0x4B0)
 	{		flashType=1;
	 
   }
	else if(DBGMCU_val== 0x492)
	{  
		flashType=0;
		
   }
	else 
	{
		flashType=1;
		
	
   }
	
  return (flashType);
}

static u32 PNBMASK_val (void) {

	u32 PNBMASK_val;

	if(GetFlashType()==0)
	{
		PNBMASK_val=(((((*(u32*) 0xBF907A0) & 0xFFF)*0x400))/0x2000)-1;
	}
	else
		{
			if((GetFlashSize()<0x200000)&&((FLASH->OPTR & 0x200000)==0x0))
			{
				PNBMASK_val=(((((*(u32*) 0xBF907A0) & 0xFFF)*0x400))/0x2000)-1;
			}
			else
				PNBMASK_val=(((((*(u32*)0x0BFA07A0) & 0xFFFF)*0x400)/2)/0x2000)-1;
		}
	return (PNBMASK_val);
}
#endif /* FLASH_MEM */





/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

	/*disable interrupts while programming*/
	__disable_irq();
	
	if ((GetFlashSecureMode() == 0U) || ((FLASH->OPTR & FLASH_OPTR_RDP)==FLASH_OPTR_RDP_55))    // Flash non-secure
		{			
		/* Unlock Flash */
		if ((FLASH->NSCR1 & FLASH_LOCK) != 0) {
		FLASH->NSKEYR = FLASH_KEY1;
		FLASH->NSKEYR = FLASH_KEY2;
	}
		/*Wait until the flash is ready*/
		while (FLASH->NSSR & FLASH_BSY);
	 
		}
	else                                // Flash secure
		{			
          /* Flash block-based secure */ 
          FLASH-> SECBBR1 = 0xffffffff;
          FLASH-> SECBBR2 = 0xffffffff; 
          FLASH-> SECBBR3 = 0xffffffff;
          FLASH-> SECBBR4 = 0xffffffff; 	
					if (GetFlashType() == 1U)                        /* Flash secure DUAL BANK */
					{
						/* Flash block-based secure bank2 */
						FLASH->SECBB2R1 = 0xFFFFFFFF;
						FLASH->SECBB2R2 = 0xFFFFFFFF;
						FLASH->SECBB2R3 = 0xFFFFFFFF;
						FLASH->SECBB2R4 = 0xFFFFFFFF;
					}			
          if ((FLASH->SECCR1 & FLASH_LOCK) != 0) 
					{
					/* Unlock Flash*/	
          FLASH->SECKEYR = FLASH_KEY1;
          FLASH->SECKEYR = FLASH_KEY2;
					}
          /*Wait until the flash is ready*/
          while (FLASH->SECSR & FLASH_BSY);    

			/*set SAU for device info region*/
	  	M32(0xE000EDD8)=0x0;
			M32(0xE000EDDC)=0x0BFA0700;
			M32(0xE000EDE0)=0x0BFA08E1;
			M32(0xE000EDD0)=0x1;
	  }
		
  return (0);
}

/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {

   if ((GetFlashSecureMode() == 0U)|| ((FLASH->OPTR & FLASH_OPTR_RDP)==FLASH_OPTR_RDP_55))   // Flash non-secure
		{
      FLASH->NSCR1 |= FLASH_LOCK;                            /* Lock Flash operation */
    }
		else                                // Flash secure
		{	
		  /* lock Flash*/	
		   FLASH->SECCR1 |= FLASH_LOCK;  
		}	
	DSB();		

#ifdef FLASH_OPT
  FLASH->NSCR1  = FLASH_OBL_LAUNCH;                         /* Load option bytes */
  DSB();
  while (FLASH->NSCR1 & FLASH_OBL_LAUNCH);

  FLASH->NSCR1 = FLASH_OPTLOCK;                             /* Lock option bytes operation */
  DSB();
#endif /* FLASH_OPT */

  return (0);
}

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */


int EraseChip (void) {
	
    if ((GetFlashSecureMode() == 0U) || ((FLASH->OPTR & FLASH_OPTR_RDP)==FLASH_OPTR_RDP_55))    // Flash non-secure
		{	
		 while (FLASH->NSSR & FLASH_BSY){};
			 
				 
     FLASH->NSSR  = FLASH_PGAERR;                              /* Reset Error Flags */
     
			 /* Dual-Bank Flash */
    if (GetFlashType() == 1U)
		{ 		
			FLASH->NSCR1 |= FLASH_MER1;        /* Bank A/B mass erase enabled */
		 FLASH->NSCR1 |=  FLASH_STRT;                           /* Start erase */
     while (FLASH->NSSR & FLASH_BSY);
		 FLASH->NSCR1 &= (~FLASH_MER1);
			 
		 FLASH->NSCR1 |= FLASH_MER2;        /* Bank A/B mass erase enabled */
     FLASH->NSCR1 |=  FLASH_STRT;
		 while (FLASH->NSSR & FLASH_BSY);
     FLASH->NSCR1 &= (~FLASH_MER2);	
		} 
		else 
		{
				FLASH->NSCR1 |= FLASH_MER1;        /* Bank A/B mass erase enabled */
		 FLASH->NSCR1 |=  FLASH_STRT;                           /* Start erase */
     while (FLASH->NSSR & FLASH_BSY);
		 FLASH->NSCR1 &= (~FLASH_MER1);
		}

			 
			 DSB();
 
   				 
		}		
	else{                                // Flash secure
		
		while (FLASH->SECSR & FLASH_BSY){};
	  FLASH->SECSR  = FLASH_PGAERR;                              /* Reset Error Flags */
		 /* Dual-Bank Flash */
    if (GetFlashType() == 1U)
		{ 
		FLASH->SECCR1 |= FLASH_MER1;        /* Bank A/B mass erase enabled */
		 FLASH->SECCR1 |=  FLASH_STRT;                           /* Start erase */
     while (FLASH->SECSR & FLASH_BSY);
		 FLASH->SECCR1 &= (~FLASH_MER1);
			 
		 FLASH->SECCR1 |= FLASH_MER2;        /* Bank A/B mass erase enabled */
     FLASH->SECCR1 |=  FLASH_STRT;
		 while (FLASH->SECSR & FLASH_BSY);
     FLASH->SECCR1 &= (~FLASH_MER2);	
		}
		else
		{
			FLASH->SECCR1 |= FLASH_MER1;        /* Bank A/B mass erase enabled */
		 FLASH->SECCR1 |=  FLASH_STRT;                           /* Start erase */
     while (FLASH->SECSR & FLASH_BSY);
		 FLASH->SECCR1 &= (~FLASH_MER1);
			
		}
			
//    FLASH->SECCR1  = (FLASH_MER1 | FLASH_MER2);             /* Bank A/B mass rrase enabled */
//    FLASH->SECCR1 |=  FLASH_STRT;                           /* Start erase */
    DSB();

					
	}
  return (0);                                            /* Done */
}


#ifdef FLASH_OPT
int EraseChip (void) {

  FLASH->NSSR  = FLASH_PGERR;                              /* Reset Error Flags */

  FLASH->OPTR     = 0xFFEFF8AA;
  FLASH->PCROP1SR = 0x000000FF;
  FLASH->PCROP1ER = 0x00000000;
  FLASH->WRP1AR   = 0x000000FF;
  FLASH->WRP1BR   = 0x000000FF;
  FLASH->SEC1R    = 0x00000000;
  if (GetFlashType() == 1U) {
  FLASH->PCROP1SR = 0x000000FF;
  FLASH->PCROP1ER = 0x00000000;
  FLASH->WRP1AR   = 0x000000FF;
  FLASH->WRP1BR   = 0x000000FF;
  FLASH->SEC1R    = 0x00000000;
  }

  FLASH->NSCR       = FLASH_OPTSTRT;                       /* Program values */
  DSB();

  while (FLASH->NSSR & FLASH_BSY);

  if (FLASH->NSSR & FLASH_PGERR) {                         /* Check for Error */
    FLASH->NSSR  = FLASH_PGERR;                            /* Reset Error Flags */
    return (1);                                          /* Failed */
  }

  return (0);                                            /* Done */
}
#endif /* FLASH_OPT */


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */


int EraseSector (unsigned long adr) {
  unsigned long page;

	if (adr >= 0x0C000000)
	{
		adr = adr - 0x04000000; 
	}

	
	if ((GetFlashSecureMode() == 0U) || ((FLASH->OPTR & FLASH_OPTR_RDP)==FLASH_OPTR_RDP_55)) {   // Flash non-secure
		{
		FLASH->NSSR  = FLASH_PGAERR;                              /* Reset Error Flags */
		/*page erase enabled*/ 
		FLASH->NSCR1 |= FLASH_PER;
		/* Dual-Bank Flash */
    if (GetFlashType() == 1U)
		{ 		
					if((GetFlashSize()<0x200000)&&((FLASH->OPTR & 0x200000)==0x0))
					{
						page = ((adr >>13) & PNBMASK_val() );   
					}
					else
					{
					 if(adr >= GetFlashBank()) // bank 2
						{
						page = (adr >>13);
						page = page - (PNBMASK_val()+1);
						page = page & PNBMASK_val();
						/*Bank 2 Erase Selection*/
						FLASH->NSCR1 |= (FLASH_BKER);           
						}    
				else               // bank 1
						{
						 page = ((adr >>13) & PNBMASK_val() );
						/*Bank 1 Erase Selection*/
						 FLASH->NSCR1 &= ~(FLASH_BKER);           
						}

		} 
	}
		else   /* Single-Bank Flash */
		{
			page = ((adr >>13) & PNBMASK_val() );   

		}			
		
		/*set PNB*/				
    FLASH->NSCR1 &= ~(PNBMASK_val()<<3);
		FLASH->NSCR1 |= page << 3;
		
    /*Start erase operation*/ 				
    FLASH->NSCR1 |= FLASH_STRT;
		
	  /*wait until the operation ends*/				
    while (FLASH->NSSR & FLASH_BSY); 

		
    /*check for error*/
    if ((FLASH->NSSR & FLASH_OPERR))    
    {
    return (1); 
    }		
    /*reset CR*/
    FLASH->NSCR1 &= (~FLASH_PER);		
		FLASH->NSCR1 &= ~(PNBMASK_val()<<3);
		}		
	}
		
	else            // Flash secure
	{
		FLASH->SECSR  = FLASH_PGAERR;                              /* Reset Error Flags */
		/*page erase enabled*/ 
		FLASH->SECCR1 |= FLASH_PER;
	
  if (GetFlashType() == 1U)
		{
			if((GetFlashSize()<0x200000)&&((FLASH->OPTR & 0x200000)==0x0))
					{
						page = ((adr >>13) & PNBMASK_val() );   
					}
			else
					{
					if(adr >= GetFlashBank()) // bank 2
							{
							page = (adr >>13);
							page = page - (PNBMASK_val()+1);
							page = page & PNBMASK_val();
							/*Bank 2 Erase Selection*/          
							FLASH->SECCR1 |= (FLASH_BKER);           
							}
					else        // bank 1
							{
							page = ((adr >>13) & PNBMASK_val() );
							/*Bank 1 Erase Selection*/
							FLASH->SECCR1 &= ~(FLASH_BKER);           
							}
						}
					}					
		else   /* Single-Bank Flash */
		{
		    page = ((adr >>13) & PNBMASK_val() );   
		
		}			
				
		
		/*set PNB*/
    FLASH->SECCR1 &= ~(PNBMASK_val()<<3);
    FLASH->SECCR1 |= page << 3;		
				 
    /*Start the erase operation*/  
    FLASH->SECCR1 |= FLASH_STRT;
	
	  /*wait until the operation ends*/	
    while (FLASH->SECSR & FLASH_BSY); 				 
	
    /*check for error*/
  if ((FLASH->SECSR & FLASH_OPERR))    
    {
    return (1); 
    }	
    /*reset CR*/
  FLASH->SECCR1 &= (~FLASH_PER);
 FLASH->SECCR1 &= ~(PNBMASK_val()<<3);
	}
	
  return (0);                                           
}


#if defined FLASH_OPT || defined FLASH_OTP
int EraseSector (unsigned long adr) {
  /* erase sector is not needed for
     - Flash Option bytes
     - Flash One Time Programmable bytes
  */
  return (0);                                            /* Done */
}
#endif /* FLASH_OPT || defined FLASH_OTP */


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */


int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
   unsigned char tab[8];
   int i;
  unsigned long align = adr%8;
  unsigned long tempadd = 0;
  sz = (sz + 15) & ~15;                                    /* Adjust size for two Doublewords */

  
 if ((GetFlashSecureMode() == 0U) || ((FLASH->OPTR & FLASH_OPTR_RDP)==FLASH_OPTR_RDP_55))        // Flash non-secure
 {
  FLASH->NSCR1 = FLASH_PG ;	                             /* Programming Enabled */

  while (sz) {
         if(sz>=8)
      {
        FLASH->NSSR  = FLASH_PGAERR;
          if (align !=0)
      {
         tempadd = (adr -align);
            for(i=0;i<align;i++)
        {
          tab[i]= *((unsigned char *)tempadd);
          tempadd=tempadd+1;
        }        
          for(i=0;i<8-align;i++)
        {
          tab[i+align]=*((unsigned char *)buf);
          buf=buf+1;
        }
             tempadd = (adr -align);
      while (FLASH->NSSR  & FLASH_BSY );
   /*write double world*/
    M32(tempadd ) = *((u32 *)tab );                   // Program the first word of the Double Word
    M32(tempadd + 4) = *((u32 *)(tab + 4)); 
        __asm volatile("NOP");
        __asm volatile("NOP"); 
       
               sz =sz -(8-align);               
               adr= adr +(8-align);
               align = 0;                
  }      
      while (FLASH->NSSR & FLASH_BSY );
   /*write double world*/
      M32(adr ) = *((u32 *)buf );                   // Program the first word of the Double Word
      M32(adr + 4) = *((u32 *)(buf + 4)); 
        buf +=8; 
        sz -= 8;
        adr +=8;
        __asm volatile("NOP");
        __asm volatile("NOP");            
      }    
      else{ 
         /*write a word completed with 0xFF*/         

        for(i=0;i<sz;i++)
        {
          tab[i]= *((unsigned char *)buf);
          buf=buf+1;
        }        
         for(i=0;i<8-sz;i++)
        {
          tab[i+sz]=0xFF;
        }
      while (FLASH->NSSR & FLASH_BSY){};
  
   
        M32(adr ) = *((u32 *)tab );                   // Program the first word of the Double Word
        M32(adr + 4) = *((u32 *)(tab + 4)); 
				
         sz =0;            
      }      
        /*wait until the operation ends*/
      while (FLASH->NSSR & FLASH_BSY );
      if (FLASH->NSSR & FLASH_PGERR) {                       /* Check for Error */
      FLASH->NSSR  = FLASH_PGERR;                          /* Reset Error Flags */
      return (1);                                        /* Failed */
    }
		
  }

  FLASH->NSCR1 = 0U;                                        /* Reset CR */
}
 else {                    // Flash secure
	 FLASH->SECSR  = FLASH_PGERR;                              /* Reset Error Flags */
  FLASH->SECCR1 = FLASH_PG ;	                             /* Programming Enabled */

  while (sz) {
  
		   if(sz>=8)
      {
        FLASH->SECSR  = 0x000001FF;
          if (align !=0)
      {
         tempadd = (adr -align);
            for(i=0;i<align;i++)
        {
          tab[i]= *((unsigned char *)tempadd);
          tempadd=tempadd+1;
        }        
          for(i=0;i<8-align;i++)
        {
          tab[i+align]=*((unsigned char *)buf);
          buf=buf+1;
        }
             tempadd = (adr -align);
      while (FLASH->SECSR  & FLASH_BSY );
   /*write double world*/
    M32(tempadd ) = *((u32 *)tab );                   // Program the first word of the Double Word
    M32(tempadd + 4) = *((u32 *)(tab + 4)); 
        __asm volatile("NOP");
        __asm volatile("NOP"); 
       
               sz =sz -(8-align);               
               adr= adr +(8-align);
               align = 0;                
  }      
      while (FLASH->SECSR & FLASH_BSY );
   /*write double world*/
      M32(adr ) = *((u32 *)buf );                   // Program the first word of the Double Word
      M32(adr + 4) = *((u32 *)(buf + 4)); 
        buf +=8; 
        sz -= 8;
        adr +=8;
        __asm volatile("NOP");
        __asm volatile("NOP");            
      }    
      else{ 
         /*write a word completed with 0xFF*/         

        for(i=0;i<sz;i++)
        {
          tab[i]= *((unsigned char *)buf);
          buf=buf+1;
        }        
         for(i=0;i<8-sz;i++)
        {
          tab[i+sz]=0xFF;
        }
      while (FLASH->SECSR & FLASH_BSY){};
  
   
        M32(adr ) = *((u32 *)tab );                   // Program the first word of the Double Word
        M32(adr + 4) = *((u32 *)(tab + 4)); 
				
         sz =0;            
      }      
        /*wait until the operation ends*/
      while (FLASH->SECSR & FLASH_BSY );
      if (FLASH->SECSR & FLASH_PGERR) {                       /* Check for Error */
      FLASH->SECSR  = FLASH_PGERR;                          /* Reset Error Flags */
      return (1);                                        /* Failed */
    }
		
	
  }

  FLASH->SECCR1 = 0U;                                        /* Reset CR */
 }
  return (0);
}

