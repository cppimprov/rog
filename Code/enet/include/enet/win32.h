/** 
 @file  win32.h
 @brief ENet Win32 header
*/
#ifndef __ENET_WIN32_H__
#define __ENET_WIN32_H__

#ifdef _MSC_VER
#ifdef ENET_BUILDING_LIB
#pragma warning (disable: 4267) // size_t to int conversion
#pragma warning (disable: 4244) // 64bit to 32bit int
#pragma warning (disable: 4018) // signed/unsigned mismatch
#pragma warning (disable: 4146) // unary minus operator applied to unsigned type
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif


#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS	 // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES	 // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES		  // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS	  // SM_*
#define NOMENUS			  // MF_*
#define NOICONS			  // IDI_*
#define NOKEYSTATES		  // MK_*
#define NOSYSCOMMANDS	 // SC_*
#define NORASTEROPS		  // Binary and Tertiary raster ops
#define NOSHOWWINDOW	  // SW_*
#define OEMRESOURCE		  // OEM Resource values
#define NOATOM			  // Atom Manager routines
#define NOCLIPBOARD		  // Clipboard routines
#define NOCOLOR			  // Screen colors
#define NOCTLMGR		  // Control and Dialog routines
#define NODRAWTEXT		  // DrawText() and DT_*
#define NOGDI			  // All GDI defines and routines
#define NOKERNEL		  // All KERNEL defines and routines
#define NOUSER			  // All USER defines and routines
#define NONLS			  // All NLS defines and routines
#define NOMB			  // MB_* and MessageBox()
#define NOMEMMGR		  // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE		  // typedef METAFILEPICT
#define NOMINMAX		  // Macros min(a, b) and max(a, b)
#define NOMSG			  // typedef MSG and associated routines
#define NOOPENFILE		  // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL		  // SB_* and scrolling routines
#define NOSERVICE		  // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND			  // Sound driver routines
#define NOTEXTMETRIC	  // typedef TEXTMETRIC and associated routines
#define NOWH			  // SetWindowsHook and WH_*
#define NOWINOFFSETS	  // GWL_*, GCL_*, associated routines
#define NOCOMM			  // COMM driver routines
#define NOKANJI			  // Kanji support stuff.
#define NOHELP			  // Help engine interface.
#define NOPROFILER		  // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX			  // Modem Configuration Extensions


#include <stdlib.h>
#include <winsock2.h>


#undef WIN32_LEAN_AND_MEAN

#undef NOGDICAPMASKS	 // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#undef NOVIRTUALKEYCODES // VK_*
#undef NOWINMESSAGES	 // WM_*, EM_*, LB_*, CB_*
#undef NOWINSTYLES		 // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#undef NOSYSMETRICS		 // SM_*
#undef NOMENUS			 // MF_*
#undef NOICONS			 // IDI_*
#undef NOKEYSTATES		 // MK_*
#undef NOSYSCOMMANDS	 // SC_*
#undef NORASTEROPS		 // Binary and Tertiary raster ops
#undef NOSHOWWINDOW		 // SW_*
#undef OEMRESOURCE		 // OEM Resource values
#undef NOATOM			 // Atom Manager routines
#undef NOCLIPBOARD		 // Clipboard routines
#undef NOCOLOR			 // Screen colors
#undef NOCTLMGR			 // Control and Dialog routines
#undef NODRAWTEXT		 // DrawText() and DT_*
#undef NOGDI			 // All GDI defines and routines
#undef NOKERNEL			 // All KERNEL defines and routines
#undef NOUSER			 // All USER defines and routines
#undef NONLS			 // All NLS defines and routines
#undef NOMB				 // MB_* and MessageBox()
#undef NOMEMMGR			 // GMEM_*, LMEM_*, GHND, LHND, associated routines
#undef NOMETAFILE		 // typedef METAFILEPICT
#undef NOMINMAX			 // Macros min(a, b) and max(a, b)
#undef NOMSG			 // typedef MSG and associated routines
#undef NOOPENFILE		 // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#undef NOSCROLL			 // SB_* and scrolling routines
#undef NOSERVICE		 // All Service Controller routines, SERVICE_ equates, etc.
#undef NOSOUND			 // Sound driver routines
#undef NOTEXTMETRIC		 // typedef TEXTMETRIC and associated routines
#undef NOWH				 // SetWindowsHook and WH_*
#undef NOWINOFFSETS		 // GWL_*, GCL_*, associated routines
#undef NOCOMM			 // COMM driver routines
#undef NOKANJI			 // Kanji support stuff.
#undef NOHELP			 // Help engine interface.
#undef NOPROFILER		 // Profiler interface.
#undef NODEFERWINDOWPOS  // DeferWindowPos routines
#undef NOMCX			 // Modem Configuration Extensions

typedef SOCKET ENetSocket;

#define ENET_SOCKET_NULL INVALID_SOCKET

#define ENET_HOST_TO_NET_16(value) (htons (value))
#define ENET_HOST_TO_NET_32(value) (htonl (value))

#define ENET_NET_TO_HOST_16(value) (ntohs (value))
#define ENET_NET_TO_HOST_32(value) (ntohl (value))

typedef struct
{
    size_t dataLength;
    void * data;
} ENetBuffer;

#define ENET_CALLBACK __cdecl

#ifdef ENET_DLL
#ifdef ENET_BUILDING_LIB
#define ENET_API __declspec( dllexport )
#else
#define ENET_API __declspec( dllimport )
#endif /* ENET_BUILDING_LIB */
#else /* !ENET_DLL */
#define ENET_API extern
#endif /* ENET_DLL */

typedef fd_set ENetSocketSet;

#define ENET_SOCKETSET_EMPTY(sockset)          FD_ZERO (& (sockset))
#define ENET_SOCKETSET_ADD(sockset, socket)    FD_SET (socket, & (sockset))
#define ENET_SOCKETSET_REMOVE(sockset, socket) FD_CLR (socket, & (sockset))
#define ENET_SOCKETSET_CHECK(sockset, socket)  FD_ISSET (socket, & (sockset))

#endif /* __ENET_WIN32_H__ */


