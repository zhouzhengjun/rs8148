/*
 *  ======== tistdtypes.h ========
 */

/*
 * These types are also defined by other TI components.  They are bracketed
 * with _TI_STD_TYPES to avoid warnings for duplicate definition.
 *
 * You may get warnings about duplicate type definitions when using this
 * header file with earlier versions of DSP/BIOS and CSL.
 *
 * You can use the '-pds303' compiler option to suppress these warnings.
 */
#ifndef _TI_STD_TYPES
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#define _TI_STD_TYPES

/* 
  * Normally the NULL, FALSE, TRUE definitions are part of std.h provided in CCStudio for c64x target
  * Pulled these into tistdtypes.h for ARM based CPU such as DM365/DM355 .
  */
#undef NULL
#define NULL 0

#undef FALSE
#define FALSE 0

#undef TRUE
#define TRUE 1

/*
 * This '#ifndef STD_' is needed to protect from duplicate definitions
 * of Int, Uns, etc. in DSP/BIOS v4.x (e.g. 4.90, 4.80) since these versions
 * of DSP/BIOS did not contain the '#ifndef_TI_STD_TYPES' logic.
 */
#ifndef STD_

/*
 * Aliases for standard C types
 */
typedef int			Int;
typedef unsigned		Uns;
typedef char			Char;

/* pointer to null-terminated character sequence */
typedef char			*String;

typedef void			*Ptr;		/* pointer to arbitrary type */

typedef unsigned short		Bool;		/* boolean */

#endif /* STD_ */

/*
 * Uint8, Uint16, Uint32, etc are defined to be "smallest unit of
 * available storage that is large enough to hold unsigned or integer
 * of specified size".
 */

/* Handle the 6x ISA */
#if 1 //defined(_TMS320C6X) || defined(_TMS470R1X) || defined(_TMS470R2X) || defined(_TMS470R3X) || defined (_DM365) || defined (_DM355)
    /* Unsigned integer definitions (32bit, 16bit, 8bit) follow... */
    typedef unsigned long long Uint64;
    typedef unsigned int	Uint32;
    typedef unsigned short	Uint16;
    typedef unsigned char	Uint8;

    /* Signed integer definitions (32bit, 16bit, 8bit) follow... */
    typedef long long Int64;
    typedef int			Int32;
    typedef short		Int16;
    typedef char		Int8;

    typedef void            Void;

/* Handle the 54x, 55x and 28x ISAs */
#elif defined(_TMS320C5XX) || defined(__TMS320C55X__) || defined(_TMS320C28X)
    /* Unsigned integer definitions (32bit, 16bit, 8bit) follow... */
    typedef unsigned long	Uint32;
    typedef unsigned short	Uint16;
    typedef unsigned char	Uint8;

    /* Signed integer definitions (32bit, 16bit, 8bit) follow... */
    typedef long		Int32;
    typedef short		Int16;
    typedef char		Int8;

#else
    /* Other ISAs not supported */
    #error <tistdtypes.h> is not supported for this target
#endif  /* defined(_6x_) */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* _TI_STD_TYPES */
