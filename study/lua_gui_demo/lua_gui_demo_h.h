

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for lua_gui_demo.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __lua_gui_demo_h_h__
#define __lua_gui_demo_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __Ilua_gui_demo_FWD_DEFINED__
#define __Ilua_gui_demo_FWD_DEFINED__
typedef interface Ilua_gui_demo Ilua_gui_demo;

#endif 	/* __Ilua_gui_demo_FWD_DEFINED__ */


#ifndef __lua_gui_demo_FWD_DEFINED__
#define __lua_gui_demo_FWD_DEFINED__

#ifdef __cplusplus
typedef class lua_gui_demo lua_gui_demo;
#else
typedef struct lua_gui_demo lua_gui_demo;
#endif /* __cplusplus */

#endif 	/* __lua_gui_demo_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __lua_gui_demo_LIBRARY_DEFINED__
#define __lua_gui_demo_LIBRARY_DEFINED__

/* library lua_gui_demo */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_lua_gui_demo;

#ifndef __Ilua_gui_demo_DISPINTERFACE_DEFINED__
#define __Ilua_gui_demo_DISPINTERFACE_DEFINED__

/* dispinterface Ilua_gui_demo */
/* [uuid] */ 


EXTERN_C const IID DIID_Ilua_gui_demo;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("C720E8CB-F20E-427B-8D0F-50FBB9279154")
    Ilua_gui_demo : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct Ilua_gui_demoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Ilua_gui_demo * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Ilua_gui_demo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Ilua_gui_demo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Ilua_gui_demo * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Ilua_gui_demo * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Ilua_gui_demo * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Ilua_gui_demo * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } Ilua_gui_demoVtbl;

    interface Ilua_gui_demo
    {
        CONST_VTBL struct Ilua_gui_demoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Ilua_gui_demo_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define Ilua_gui_demo_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define Ilua_gui_demo_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define Ilua_gui_demo_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define Ilua_gui_demo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define Ilua_gui_demo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define Ilua_gui_demo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __Ilua_gui_demo_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_lua_gui_demo;

#ifdef __cplusplus

class DECLSPEC_UUID("74EFA0ED-FA22-45B1-B6E5-F724269C1ED3")
lua_gui_demo;
#endif
#endif /* __lua_gui_demo_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


