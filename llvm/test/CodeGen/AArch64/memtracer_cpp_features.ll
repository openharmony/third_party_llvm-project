; REQUIRES: aarch64-registered-target

; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj %s -O2 -o %t.o
; RUN: ld.lld -shared %t.o -o %t.o
; RUN: llvm-dwarfdump --mem_tracer %t.o | FileCheck %s

; CHECK: .mem_tracer contents:
; CHECK: var="src" type="void*"
; CHECK: var="dst" type="void*"
; CHECK: var="base" type="Base*"
; CHECK: var="base->data" type="char*"
; CHECK: var="derived" type="Derived*"
; CHECK: var="obj->data3" type="char*"

; ModuleID = 'memtracer_cpp_features.cpp'
source_filename = "memtracer_cpp_features.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-ohos"

%"class.std::__h::vector" = type { ptr, ptr, %"class.std::__h::__compressed_pair" }
%"class.std::__h::__compressed_pair" = type { %"struct.std::__h::__compressed_pair_elem" }
%"struct.std::__h::__compressed_pair_elem" = type { ptr }
%"class.std::__h::__shared_count" = type { ptr, i64 }
%"class.std::__h::__shared_ptr_pointer" = type { %"class.std::__h::__shared_weak_count", %"class.std::__h::__compressed_pair.7" }
%"class.std::__h::__shared_weak_count" = type { %"class.std::__h::__shared_count", i64 }
%"class.std::__h::__compressed_pair.7" = type { %"struct.std::__h::__compressed_pair_elem.8" }
%"struct.std::__h::__compressed_pair_elem.8" = type { %"class.std::__h::__compressed_pair.1" }
%"class.std::__h::__compressed_pair.1" = type { %"struct.std::__h::__compressed_pair_elem.2", %"struct.std::__h::__compressed_pair_elem.3" }
%"struct.std::__h::__compressed_pair_elem.2" = type { ptr }
%"struct.std::__h::__compressed_pair_elem.3" = type { ptr }
%class.Base = type { ptr, ptr }
%class.Derived = type { %class.Base, ptr }
%class.MultiDerived = type { %class.Base1, %class.Base2, ptr }
%class.Base1 = type { ptr }
%class.Base2 = type { ptr }
%"class.std::type_info" = type { ptr, ptr }

$_ZN4BaseD0Ev = comdat any

$_ZN4BaseD2Ev = comdat any

$_ZN7DerivedD0Ev = comdat any

$__clang_call_terminate = comdat any

$_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEED0Ev = comdat any

$_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE16__on_zero_sharedEv = comdat any

$_ZNKSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE13__get_deleterERKSt9type_info = comdat any

$_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE21__on_zero_shared_weakEv = comdat any

$_ZTV4Base = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTI7Derived = comdat any

$_ZTVNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE = comdat any

$_ZTSNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE = comdat any

$_ZTINSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE = comdat any

$_ZTSPFvPvE = comdat any

@.str = private unnamed_addr constant [6 x i8] c"hello\00", align 1, !dbg !0
@_ZL6g_sink = internal unnamed_addr global [32 x ptr] zeroinitializer, align 8, !dbg !8
@_ZL10g_sink_idx = internal unnamed_addr global i32 0, align 4, !dbg !1345
@_ZTV4Base = linkonce_odr unnamed_addr constant { [4 x ptr] } { [4 x ptr] [ptr null, ptr @_ZTI4Base, ptr @_ZN4BaseD2Ev, ptr @_ZN4BaseD0Ev] }, comdat, align 8
@_ZTVN10__cxxabiv117__class_type_infoE = external global ptr
@_ZTS4Base = linkonce_odr constant [6 x i8] c"4Base\00", comdat, align 1
@_ZTI4Base = linkonce_odr constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS4Base }, comdat, align 8
@_ZTV7Derived = linkonce_odr unnamed_addr constant { [4 x ptr] } { [4 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN4BaseD2Ev, ptr @_ZN7DerivedD0Ev] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global ptr
@_ZTS7Derived = linkonce_odr constant [9 x i8] c"7Derived\00", comdat, align 1
@_ZTI7Derived = linkonce_odr constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI4Base }, comdat, align 8
@_ZTVNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE = linkonce_odr unnamed_addr constant { [7 x ptr] } { [7 x ptr] [ptr null, ptr @_ZTINSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE, ptr @_ZNSt3__h19__shared_weak_countD2Ev, ptr @_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEED0Ev, ptr @_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE16__on_zero_sharedEv, ptr @_ZNKSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE13__get_deleterERKSt9type_info, ptr @_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE21__on_zero_shared_weakEv] }, comdat, align 8
@_ZTSNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE = linkonce_odr constant [58 x i8] c"NSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE\00", comdat, align 1
@_ZTINSt3__h19__shared_weak_countE = external constant ptr
@_ZTINSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE = linkonce_odr constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE, ptr @_ZTINSt3__h19__shared_weak_countE }, comdat, align 8
@_ZTSPFvPvE = linkonce_odr constant [7 x i8] c"PFvPvE\00", comdat, align 1

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z17test_memcpy_storev() local_unnamed_addr #0 !dbg !2046 {
  %1 = tail call dereferenceable_or_null(256) ptr @malloc(i64 noundef 256) #20, !dbg !2050, !memtracer !2051
  call void @llvm.dbg.value(metadata ptr %1, metadata !2048, metadata !DIExpression()), !dbg !2052
  %2 = tail call dereferenceable_or_null(256) ptr @malloc(i64 noundef 256) #20, !dbg !2053, !memtracer !2054
  call void @llvm.dbg.value(metadata ptr %2, metadata !2049, metadata !DIExpression()), !dbg !2052
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(256) %2, ptr noundef nonnull align 1 dereferenceable(256) %1, i64 256, i1 false), !dbg !2055
  call void @llvm.dbg.value(metadata ptr %1, metadata !2056, metadata !DIExpression()), !dbg !2059
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2061, !tbaa !2062
  %4 = add nsw i32 %3, 1, !dbg !2061
  %5 = sext i32 %3 to i64, !dbg !2066
  %6 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !2066
  store ptr %1, ptr %6, align 8, !dbg !2067, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %2, metadata !2056, metadata !DIExpression()), !dbg !2071
  %7 = add nsw i32 %3, 2, !dbg !2073
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !2073, !tbaa !2062
  %8 = sext i32 %4 to i64, !dbg !2074
  %9 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !2074
  store ptr %2, ptr %9, align 8, !dbg !2075, !tbaa !2068, !memtracer !2070
  ret void, !dbg !2076
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #3

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define noundef ptr @_Z17test_strcpy_storePKc(ptr nocapture noundef readonly %0) local_unnamed_addr #0 !dbg !2077 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2079, metadata !DIExpression()), !dbg !2081
  %2 = tail call dereferenceable_or_null(256) ptr @malloc(i64 noundef 256) #20, !dbg !2082, !memtracer !2083
  call void @llvm.dbg.value(metadata ptr %2, metadata !2080, metadata !DIExpression()), !dbg !2081
  %3 = tail call ptr @strcpy(ptr noundef nonnull dereferenceable(1) %2, ptr noundef nonnull dereferenceable(1) %0) #21, !dbg !2084
  call void @llvm.dbg.value(metadata ptr %2, metadata !2056, metadata !DIExpression()), !dbg !2085
  %4 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2087, !tbaa !2062
  %5 = add nsw i32 %4, 1, !dbg !2087
  store i32 %5, ptr @_ZL10g_sink_idx, align 4, !dbg !2087, !tbaa !2062
  %6 = sext i32 %4 to i64, !dbg !2088
  %7 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %6, !dbg !2088
  store ptr %2, ptr %7, align 8, !dbg !2089, !tbaa !2068, !memtracer !2070
  ret ptr %2, !dbg !2090
}

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare ptr @strcpy(ptr noalias noundef returned writeonly, ptr noalias nocapture noundef readonly) local_unnamed_addr #5

; Function Attrs: noinline
define void @_Z21test_stl_vector_storev() local_unnamed_addr #6 personality ptr @__gxx_personality_v0 !dbg !2091 {
  %1 = alloca %"class.std::__h::vector", align 8
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %1) #22, !dbg !2094
  call void @llvm.dbg.declare(metadata ptr %1, metadata !2093, metadata !DIExpression()), !dbg !2095
  call void @llvm.dbg.value(metadata ptr %1, metadata !2096, metadata !DIExpression()), !dbg !2099
  %2 = getelementptr inbounds i8, ptr %1, i64 8, !dbg !2101
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %2, i8 0, i64 16, i1 false), !dbg !2101
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2102, !memtracer !2103
  call void @llvm.dbg.value(metadata ptr %1, metadata !2104, metadata !DIExpression()), !dbg !2108
  call void @llvm.dbg.value(metadata ptr undef, metadata !2107, metadata !DIExpression()), !dbg !2108
  call void @llvm.dbg.value(metadata ptr %1, metadata !2110, metadata !DIExpression()), !dbg !2113
  call void @llvm.dbg.value(metadata ptr %1, metadata !2116, metadata !DIExpression()), !dbg !2128
  call void @llvm.dbg.value(metadata ptr undef, metadata !2125, metadata !DIExpression()), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !2126, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr undef, metadata !2130, metadata !DIExpression()), !dbg !2147
  call void @llvm.dbg.value(metadata i64 1, metadata !2133, metadata !DIExpression()), !dbg !2147
  call void @llvm.dbg.value(metadata i64 0, metadata !2134, metadata !DIExpression()), !dbg !2147
  call void @llvm.dbg.value(metadata ptr %1, metadata !2135, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2147
  call void @llvm.dbg.value(metadata ptr null, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value, DW_OP_LLVM_fragment, 256, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr undef, metadata !2149, metadata !DIExpression()), !dbg !2155
  call void @llvm.dbg.value(metadata i64 1, metadata !2154, metadata !DIExpression()), !dbg !2155
  call void @llvm.dbg.value(metadata ptr undef, metadata !2157, metadata !DIExpression()), !dbg !2162
  call void @llvm.dbg.value(metadata i64 1, metadata !2160, metadata !DIExpression()), !dbg !2162
  call void @llvm.dbg.value(metadata i64 8, metadata !2164, metadata !DIExpression()), !dbg !2171
  call void @llvm.dbg.value(metadata i64 8, metadata !2170, metadata !DIExpression()), !dbg !2171
  call void @llvm.dbg.value(metadata i64 8, metadata !2175, metadata !DIExpression()), !dbg !2184
  %4 = tail call noalias noundef nonnull dereferenceable(8) ptr @_Znwm(i64 noundef 8) #23, !dbg !2186
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata i64 1, metadata !2133, metadata !DIExpression()), !dbg !2147
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 128, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value, DW_OP_LLVM_fragment, 192, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !2187, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2200
  call void @llvm.dbg.value(metadata ptr %4, metadata !2198, metadata !DIExpression()), !dbg !2200
  call void @llvm.dbg.value(metadata ptr undef, metadata !2199, metadata !DIExpression()), !dbg !2200
  call void @llvm.dbg.value(metadata ptr %1, metadata !2202, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2211
  call void @llvm.dbg.value(metadata ptr %4, metadata !2209, metadata !DIExpression()), !dbg !2211
  call void @llvm.dbg.value(metadata ptr undef, metadata !2210, metadata !DIExpression()), !dbg !2211
  store ptr %3, ptr %4, align 8, !dbg !2213, !tbaa !2068, !memtracer !2214
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value, DW_OP_LLVM_fragment, 128, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !697, metadata !DIExpression()), !dbg !2215
  call void @llvm.dbg.value(metadata ptr undef, metadata !699, metadata !DIExpression()), !dbg !2215
  call void @llvm.dbg.value(metadata i64 undef, metadata !2217, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2230
  call void @llvm.dbg.value(metadata i64 undef, metadata !2217, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2230
  call void @llvm.dbg.value(metadata i64 undef, metadata !2224, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2230
  call void @llvm.dbg.value(metadata i64 undef, metadata !2224, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2230
  call void @llvm.dbg.value(metadata i64 undef, metadata !2225, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2230
  call void @llvm.dbg.value(metadata i64 undef, metadata !2225, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2230
  call void @llvm.dbg.value(metadata ptr %1, metadata !2223, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2230
  call void @llvm.dbg.value(metadata i64 undef, metadata !2232, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2243
  call void @llvm.dbg.value(metadata i64 undef, metadata !2232, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2243
  call void @llvm.dbg.value(metadata i64 undef, metadata !2238, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2243
  call void @llvm.dbg.value(metadata i64 undef, metadata !2238, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2243
  call void @llvm.dbg.value(metadata i64 undef, metadata !2239, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2243
  call void @llvm.dbg.value(metadata i64 undef, metadata !2239, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2243
  call void @llvm.dbg.value(metadata i64 undef, metadata !2247, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata i64 undef, metadata !2247, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata i64 undef, metadata !2257, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata i64 undef, metadata !2257, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata i64 undef, metadata !2258, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata i64 undef, metadata !2258, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata i64 undef, metadata !2269, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2295
  call void @llvm.dbg.value(metadata i64 0, metadata !2269, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2295
  call void @llvm.dbg.value(metadata i64 undef, metadata !2274, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2295
  call void @llvm.dbg.value(metadata ptr null, metadata !2274, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2295
  call void @llvm.dbg.value(metadata i64 undef, metadata !2275, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2295
  call void @llvm.dbg.value(metadata ptr %4, metadata !2275, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2295
  call void @llvm.dbg.value(metadata i64 0, metadata !2276, metadata !DIExpression()), !dbg !2295
  call void @llvm.dbg.value(metadata ptr null, metadata !2288, metadata !DIExpression()), !dbg !2295
  call void @llvm.dbg.value(metadata ptr %4, metadata !2289, metadata !DIExpression()), !dbg !2295
  call void @llvm.dbg.value(metadata ptr %4, metadata !2290, metadata !DIExpression()), !dbg !2295
  call void @llvm.dbg.value(metadata ptr null, metadata !2297, metadata !DIExpression()), !dbg !2309
  call void @llvm.dbg.value(metadata i64 0, metadata !2302, metadata !DIExpression()), !dbg !2309
  call void @llvm.dbg.value(metadata ptr %4, metadata !2303, metadata !DIExpression()), !dbg !2309
  call void @llvm.dbg.value(metadata i64 0, metadata !2304, metadata !DIExpression()), !dbg !2309
  call void @llvm.dbg.value(metadata i64 undef, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata ptr null, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata ptr %4, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 128, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata ptr %4, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !2267
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !2311, metadata !DIExpression()), !dbg !2325
  call void @llvm.dbg.value(metadata ptr undef, metadata !2322, metadata !DIExpression()), !dbg !2325
  call void @llvm.dbg.value(metadata ptr null, metadata !2323, metadata !DIExpression()), !dbg !2325
  store ptr %4, ptr %1, align 8, !dbg !2327, !tbaa !2068, !memtracer !2328
  call void @llvm.dbg.value(metadata ptr null, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !2311, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value)), !dbg !2329
  call void @llvm.dbg.value(metadata ptr undef, metadata !2322, metadata !DIExpression()), !dbg !2329
  call void @llvm.dbg.value(metadata ptr null, metadata !2323, metadata !DIExpression()), !dbg !2329
  call void @llvm.dbg.value(metadata ptr null, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 128, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr %1, metadata !2311, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2331
  call void @llvm.dbg.value(metadata ptr undef, metadata !2322, metadata !DIExpression()), !dbg !2331
  call void @llvm.dbg.value(metadata ptr undef, metadata !2323, metadata !DIExpression()), !dbg !2331
  call void @llvm.dbg.value(metadata ptr undef, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr null, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2128
  call void @llvm.dbg.value(metadata ptr undef, metadata !2333, metadata !DIExpression()), !dbg !2336
  %5 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2338, !memtracer !2103
  call void @llvm.dbg.value(metadata ptr %1, metadata !2104, metadata !DIExpression()), !dbg !2339
  call void @llvm.dbg.value(metadata ptr undef, metadata !2107, metadata !DIExpression()), !dbg !2339
  call void @llvm.dbg.value(metadata ptr %1, metadata !2110, metadata !DIExpression()), !dbg !2341
  call void @llvm.dbg.value(metadata ptr %1, metadata !2116, metadata !DIExpression()), !dbg !2343
  call void @llvm.dbg.value(metadata ptr undef, metadata !2125, metadata !DIExpression()), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !2126, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr undef, metadata !2130, metadata !DIExpression()), !dbg !2345
  call void @llvm.dbg.value(metadata i64 2, metadata !2133, metadata !DIExpression()), !dbg !2345
  call void @llvm.dbg.value(metadata i64 1, metadata !2134, metadata !DIExpression()), !dbg !2345
  call void @llvm.dbg.value(metadata ptr %1, metadata !2135, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2345
  call void @llvm.dbg.value(metadata ptr null, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value, DW_OP_LLVM_fragment, 256, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr undef, metadata !2149, metadata !DIExpression()), !dbg !2347
  call void @llvm.dbg.value(metadata i64 2, metadata !2154, metadata !DIExpression()), !dbg !2347
  call void @llvm.dbg.value(metadata ptr undef, metadata !2157, metadata !DIExpression()), !dbg !2349
  call void @llvm.dbg.value(metadata i64 2, metadata !2160, metadata !DIExpression()), !dbg !2349
  call void @llvm.dbg.value(metadata i64 16, metadata !2164, metadata !DIExpression()), !dbg !2351
  call void @llvm.dbg.value(metadata i64 8, metadata !2170, metadata !DIExpression()), !dbg !2351
  call void @llvm.dbg.value(metadata i64 16, metadata !2175, metadata !DIExpression()), !dbg !2353
  %6 = invoke noalias noundef nonnull dereferenceable(16) ptr @_Znwm(i64 noundef 16) #23
          to label %7 unwind label %20, !dbg !2355

7:                                                ; preds = %0
  call void @llvm.dbg.value(metadata ptr %6, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata i64 2, metadata !2133, metadata !DIExpression()), !dbg !2345
  %8 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !2356
  call void @llvm.dbg.value(metadata ptr %8, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 128, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %8, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %6, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value, DW_OP_LLVM_fragment, 192, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !2187, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2357
  call void @llvm.dbg.value(metadata ptr %8, metadata !2198, metadata !DIExpression()), !dbg !2357
  call void @llvm.dbg.value(metadata ptr undef, metadata !2199, metadata !DIExpression()), !dbg !2357
  call void @llvm.dbg.value(metadata ptr %1, metadata !2202, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2359
  call void @llvm.dbg.value(metadata ptr %8, metadata !2209, metadata !DIExpression()), !dbg !2359
  call void @llvm.dbg.value(metadata ptr undef, metadata !2210, metadata !DIExpression()), !dbg !2359
  store ptr %5, ptr %8, align 8, !dbg !2361, !tbaa !2068, !memtracer !2214
  call void @llvm.dbg.value(metadata ptr %6, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value, DW_OP_LLVM_fragment, 128, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !697, metadata !DIExpression()), !dbg !2362
  call void @llvm.dbg.value(metadata ptr undef, metadata !699, metadata !DIExpression()), !dbg !2362
  call void @llvm.dbg.value(metadata i64 undef, metadata !2217, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2364
  call void @llvm.dbg.value(metadata i64 undef, metadata !2217, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2364
  call void @llvm.dbg.value(metadata i64 undef, metadata !2224, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2364
  call void @llvm.dbg.value(metadata i64 undef, metadata !2224, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2364
  call void @llvm.dbg.value(metadata i64 undef, metadata !2225, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2364
  call void @llvm.dbg.value(metadata i64 undef, metadata !2225, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2364
  call void @llvm.dbg.value(metadata ptr %1, metadata !2223, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2364
  call void @llvm.dbg.value(metadata i64 undef, metadata !2232, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2366
  call void @llvm.dbg.value(metadata i64 undef, metadata !2232, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2366
  call void @llvm.dbg.value(metadata i64 undef, metadata !2238, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2366
  call void @llvm.dbg.value(metadata i64 undef, metadata !2238, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2366
  call void @llvm.dbg.value(metadata i64 undef, metadata !2239, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2366
  call void @llvm.dbg.value(metadata i64 undef, metadata !2239, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2366
  call void @llvm.dbg.value(metadata i64 undef, metadata !2247, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata i64 undef, metadata !2247, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata i64 undef, metadata !2257, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata i64 undef, metadata !2257, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata i64 undef, metadata !2258, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata i64 undef, metadata !2258, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata i64 undef, metadata !2269, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %4, metadata !2269, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value, DW_OP_LLVM_fragment, 64, 64)), !dbg !2370
  call void @llvm.dbg.value(metadata i64 undef, metadata !2274, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %4, metadata !2274, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2370
  call void @llvm.dbg.value(metadata i64 undef, metadata !2275, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %8, metadata !2275, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %4, metadata !2276, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value)), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %4, metadata !2288, metadata !DIExpression()), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %8, metadata !2289, metadata !DIExpression()), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %6, metadata !2290, metadata !DIExpression()), !dbg !2370
  call void @llvm.dbg.value(metadata ptr %4, metadata !2297, metadata !DIExpression()), !dbg !2372
  call void @llvm.dbg.value(metadata ptr %4, metadata !2302, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value)), !dbg !2372
  call void @llvm.dbg.value(metadata ptr %6, metadata !2303, metadata !DIExpression()), !dbg !2372
  call void @llvm.dbg.value(metadata i64 1, metadata !2304, metadata !DIExpression()), !dbg !2372
  %9 = load i64, ptr %4, align 8, !dbg !2374
  store i64 %9, ptr %6, align 8, !dbg !2374
  call void @llvm.dbg.value(metadata i64 undef, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata ptr %4, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata ptr %6, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 128, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata ptr %6, metadata !2259, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !2368
  call void @llvm.dbg.value(metadata ptr %6, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !2311, metadata !DIExpression()), !dbg !2375
  call void @llvm.dbg.value(metadata ptr undef, metadata !2322, metadata !DIExpression()), !dbg !2375
  call void @llvm.dbg.value(metadata ptr %4, metadata !2323, metadata !DIExpression()), !dbg !2375
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !2311, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value)), !dbg !2377
  call void @llvm.dbg.value(metadata ptr undef, metadata !2322, metadata !DIExpression()), !dbg !2377
  call void @llvm.dbg.value(metadata ptr %4, metadata !2323, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value)), !dbg !2377
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_plus_uconst, 8, DW_OP_stack_value, DW_OP_LLVM_fragment, 128, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %1, metadata !2311, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2379
  call void @llvm.dbg.value(metadata ptr undef, metadata !2322, metadata !DIExpression()), !dbg !2379
  call void @llvm.dbg.value(metadata ptr undef, metadata !2323, metadata !DIExpression()), !dbg !2379
  call void @llvm.dbg.value(metadata ptr undef, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr %4, metadata !2127, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2343
  call void @llvm.dbg.value(metadata ptr undef, metadata !2333, metadata !DIExpression()), !dbg !2381
  call void @llvm.dbg.value(metadata ptr undef, metadata !2383, metadata !DIExpression()), !dbg !2388
  call void @llvm.dbg.value(metadata ptr %4, metadata !2386, metadata !DIExpression()), !dbg !2388
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %4), metadata !2387, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_stack_value)), !dbg !2388
  call void @llvm.dbg.value(metadata ptr undef, metadata !2392, metadata !DIExpression()), !dbg !2397
  call void @llvm.dbg.value(metadata ptr %4, metadata !2395, metadata !DIExpression()), !dbg !2397
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %4), metadata !2396, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_stack_value)), !dbg !2397
  call void @llvm.dbg.value(metadata ptr %4, metadata !2399, metadata !DIExpression()), !dbg !2406
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %4), metadata !2404, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_constu, 3, DW_OP_shl, DW_OP_stack_value)), !dbg !2406
  call void @llvm.dbg.value(metadata i64 8, metadata !2405, metadata !DIExpression()), !dbg !2406
  call void @llvm.dbg.value(metadata ptr %4, metadata !2410, metadata !DIExpression()), !dbg !2418
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %4), metadata !2415, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_constu, 3, DW_OP_shl, DW_OP_stack_value)), !dbg !2418
  call void @llvm.dbg.value(metadata ptr %4, metadata !2420, metadata !DIExpression()), !dbg !2427
  tail call void @_ZdlPv(ptr noundef nonnull %4) #24, !dbg !2429
  call void @llvm.dbg.value(metadata ptr %1, metadata !2430, metadata !DIExpression()), !dbg !2434
  call void @llvm.dbg.value(metadata i64 0, metadata !2433, metadata !DIExpression()), !dbg !2434
  %10 = load ptr, ptr %6, align 8, !dbg !2436, !tbaa !2068
  call void @llvm.dbg.value(metadata ptr %10, metadata !2056, metadata !DIExpression()), !dbg !2437
  %11 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2439, !tbaa !2062
  %12 = add nsw i32 %11, 1, !dbg !2439
  %13 = sext i32 %11 to i64, !dbg !2440
  %14 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %13, !dbg !2440
  store ptr %10, ptr %14, align 8, !dbg !2441, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %1, metadata !2430, metadata !DIExpression()), !dbg !2442
  call void @llvm.dbg.value(metadata i64 1, metadata !2433, metadata !DIExpression()), !dbg !2442
  %15 = getelementptr inbounds ptr, ptr %6, i64 1, !dbg !2444
  %16 = load ptr, ptr %15, align 8, !dbg !2445, !tbaa !2068
  call void @llvm.dbg.value(metadata ptr %16, metadata !2056, metadata !DIExpression()), !dbg !2446
  %17 = add nsw i32 %11, 2, !dbg !2448
  store i32 %17, ptr @_ZL10g_sink_idx, align 4, !dbg !2448, !tbaa !2062
  %18 = sext i32 %12 to i64, !dbg !2449
  %19 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %18, !dbg !2449
  store ptr %16, ptr %19, align 8, !dbg !2450, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %1, metadata !2451, metadata !DIExpression()), !dbg !2454
  call void @llvm.dbg.value(metadata ptr undef, metadata !2456, metadata !DIExpression()), !dbg !2460
  call void @llvm.dbg.value(metadata ptr %1, metadata !2463, metadata !DIExpression()), !dbg !2466
  call void @llvm.dbg.value(metadata ptr %1, metadata !2470, metadata !DIExpression()), !dbg !2475
  call void @llvm.dbg.value(metadata ptr %6, metadata !2473, metadata !DIExpression()), !dbg !2475
  call void @llvm.dbg.value(metadata i32 undef, metadata !2474, metadata !DIExpression(DW_OP_constu, 8, DW_OP_minus, DW_OP_stack_value)), !dbg !2475
  call void @llvm.dbg.value(metadata ptr %1, metadata !2383, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2477
  call void @llvm.dbg.value(metadata ptr %6, metadata !2386, metadata !DIExpression()), !dbg !2477
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %6), metadata !2387, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_stack_value)), !dbg !2477
  call void @llvm.dbg.value(metadata ptr %1, metadata !2392, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2479
  call void @llvm.dbg.value(metadata ptr %6, metadata !2395, metadata !DIExpression()), !dbg !2479
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %6), metadata !2396, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_stack_value)), !dbg !2479
  call void @llvm.dbg.value(metadata ptr %6, metadata !2399, metadata !DIExpression()), !dbg !2481
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %6), metadata !2404, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_constu, 3, DW_OP_shl, DW_OP_stack_value)), !dbg !2481
  call void @llvm.dbg.value(metadata i64 8, metadata !2405, metadata !DIExpression()), !dbg !2481
  call void @llvm.dbg.value(metadata ptr %6, metadata !2410, metadata !DIExpression()), !dbg !2483
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %6), metadata !2415, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_constu, 3, DW_OP_shl, DW_OP_stack_value)), !dbg !2483
  call void @llvm.dbg.value(metadata ptr %6, metadata !2420, metadata !DIExpression()), !dbg !2485
  tail call void @_ZdlPv(ptr noundef nonnull %6) #24, !dbg !2487
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %1) #22, !dbg !2488
  ret void, !dbg !2488

20:                                               ; preds = %0
  %21 = landingpad { ptr, i32 }
          cleanup, !dbg !2488
  %22 = load ptr, ptr %1, align 8, !dbg !2489, !tbaa !2492
  call void @llvm.dbg.value(metadata ptr %1, metadata !2451, metadata !DIExpression()), !dbg !2496
  call void @llvm.dbg.value(metadata ptr undef, metadata !2456, metadata !DIExpression()), !dbg !2497
  %23 = icmp eq ptr %22, null, !dbg !2498
  br i1 %23, label %25, label %24, !dbg !2499

24:                                               ; preds = %20
  call void @llvm.dbg.value(metadata ptr %1, metadata !2463, metadata !DIExpression()), !dbg !2500
  call void @llvm.dbg.value(metadata ptr %1, metadata !2470, metadata !DIExpression()), !dbg !2502
  call void @llvm.dbg.value(metadata ptr %22, metadata !2473, metadata !DIExpression()), !dbg !2502
  call void @llvm.dbg.value(metadata i32 undef, metadata !2474, metadata !DIExpression(DW_OP_constu, 8, DW_OP_minus, DW_OP_stack_value)), !dbg !2502
  call void @llvm.dbg.value(metadata ptr %1, metadata !2383, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2504
  call void @llvm.dbg.value(metadata ptr %22, metadata !2386, metadata !DIExpression()), !dbg !2504
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %22), metadata !2387, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_stack_value)), !dbg !2504
  call void @llvm.dbg.value(metadata ptr %1, metadata !2392, metadata !DIExpression(DW_OP_plus_uconst, 16, DW_OP_stack_value)), !dbg !2506
  call void @llvm.dbg.value(metadata ptr %22, metadata !2395, metadata !DIExpression()), !dbg !2506
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %22), metadata !2396, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_stack_value)), !dbg !2506
  call void @llvm.dbg.value(metadata ptr %22, metadata !2399, metadata !DIExpression()), !dbg !2508
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %22), metadata !2404, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_constu, 3, DW_OP_shl, DW_OP_stack_value)), !dbg !2508
  call void @llvm.dbg.value(metadata i64 8, metadata !2405, metadata !DIExpression()), !dbg !2508
  call void @llvm.dbg.value(metadata ptr %22, metadata !2410, metadata !DIExpression()), !dbg !2510
  call void @llvm.dbg.value(metadata !DIArgList(ptr undef, ptr %22), metadata !2415, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_constu, 3, DW_OP_shra, DW_OP_constu, 3, DW_OP_shl, DW_OP_stack_value)), !dbg !2510
  call void @llvm.dbg.value(metadata ptr %22, metadata !2420, metadata !DIExpression()), !dbg !2512
  tail call void @_ZdlPv(ptr noundef nonnull %22) #24, !dbg !2514
  br label %25, !dbg !2515

25:                                               ; preds = %20, %24
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %1) #22, !dbg !2488
  resume { ptr, i32 } %21, !dbg !2488
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: noinline
define void @_Z20test_smart_ptr_storev() local_unnamed_addr #6 personality ptr @__gxx_personality_v0 !dbg !2516 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2520
  call void @llvm.dbg.value(metadata ptr %1, metadata !2518, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2521
  call void @llvm.dbg.value(metadata ptr @free, metadata !2518, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2521
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2522
  call void @llvm.dbg.value(metadata ptr undef, metadata !2523, metadata !DIExpression()), !dbg !2534
  call void @llvm.dbg.value(metadata ptr %2, metadata !2531, metadata !DIExpression()), !dbg !2534
  call void @llvm.dbg.value(metadata ptr @free, metadata !2532, metadata !DIExpression()), !dbg !2534
  call void @llvm.dbg.value(metadata ptr %2, metadata !2519, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2521
  %3 = invoke noalias noundef nonnull dereferenceable(40) ptr @_Znwm(i64 noundef 40) #23
          to label %14 unwind label %4, !dbg !2536, !memtracer !2539, !heapallocsite !2540

4:                                                ; preds = %0
  %5 = landingpad { ptr, i32 }
          catch ptr null, !dbg !2541
  %6 = extractvalue { ptr, i32 } %5, 0, !dbg !2541
  %7 = tail call ptr @__cxa_begin_catch(ptr %6) #22, !dbg !2542
  tail call void @free(ptr noundef %2) #21, !dbg !2543
  invoke void @__cxa_rethrow() #25
          to label %13 unwind label %8, !dbg !2545

8:                                                ; preds = %4
  %9 = landingpad { ptr, i32 }
          cleanup, !dbg !2546
  invoke void @__cxa_end_catch()
          to label %35 unwind label %10, !dbg !2547

10:                                               ; preds = %8
  %11 = landingpad { ptr, i32 }
          catch ptr null, !dbg !2547
  %12 = extractvalue { ptr, i32 } %11, 0, !dbg !2547
  tail call void @__clang_call_terminate(ptr %12) #26, !dbg !2547
  unreachable, !dbg !2547

13:                                               ; preds = %4
  unreachable

14:                                               ; preds = %0
  call void @llvm.dbg.value(metadata i8 undef, metadata !2548, metadata !DIExpression()), !dbg !2554
  call void @llvm.dbg.value(metadata ptr %3, metadata !2551, metadata !DIExpression()), !dbg !2554
  call void @llvm.dbg.value(metadata ptr %2, metadata !2552, metadata !DIExpression()), !dbg !2554
  call void @llvm.dbg.value(metadata ptr @free, metadata !2553, metadata !DIExpression()), !dbg !2554
  call void @llvm.dbg.value(metadata ptr %3, metadata !2556, metadata !DIExpression()), !dbg !2564
  call void @llvm.dbg.value(metadata i64 0, metadata !2563, metadata !DIExpression()), !dbg !2564
  call void @llvm.dbg.value(metadata ptr %3, metadata !2566, metadata !DIExpression()), !dbg !2575
  call void @llvm.dbg.value(metadata i64 0, metadata !2573, metadata !DIExpression()), !dbg !2575
  %15 = getelementptr inbounds %"class.std::__h::__shared_count", ptr %3, i64 0, i32 1, !dbg !2577
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %15, i8 0, i64 16, i1 false), !dbg !2578
  store ptr getelementptr inbounds ({ [7 x ptr] }, ptr @_ZTVNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE, i64 0, inrange i32 0, i64 2), ptr %3, align 8, !dbg !2579, !tbaa !2580, !memtracer !2582
  %16 = getelementptr inbounds %"class.std::__h::__shared_ptr_pointer", ptr %3, i64 0, i32 1, !dbg !2583
  call void @llvm.dbg.value(metadata ptr %16, metadata !2584, metadata !DIExpression()), !dbg !2597
  call void @llvm.dbg.value(metadata ptr undef, metadata !2595, metadata !DIExpression()), !dbg !2597
  call void @llvm.dbg.value(metadata ptr undef, metadata !2596, metadata !DIExpression()), !dbg !2597
  call void @llvm.dbg.value(metadata ptr %16, metadata !2599, metadata !DIExpression()), !dbg !2608
  call void @llvm.dbg.value(metadata ptr undef, metadata !2607, metadata !DIExpression()), !dbg !2608
  store ptr %2, ptr %16, align 8, !dbg !2610
  %17 = getelementptr inbounds %"class.std::__h::__shared_ptr_pointer", ptr %3, i64 0, i32 1, i32 0, i32 0, i32 1, !dbg !2610
  store ptr @free, ptr %17, align 8, !dbg !2610
  call void @llvm.dbg.value(metadata ptr %3, metadata !2519, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !2521
  call void @llvm.dbg.value(metadata ptr %1, metadata !2056, metadata !DIExpression()), !dbg !2611
  %18 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2613, !tbaa !2062
  %19 = add nsw i32 %18, 1, !dbg !2613
  %20 = sext i32 %18 to i64, !dbg !2614
  %21 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %20, !dbg !2614
  store ptr %1, ptr %21, align 8, !dbg !2615, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %2, metadata !2056, metadata !DIExpression()), !dbg !2616
  %22 = add nsw i32 %18, 2, !dbg !2618
  store i32 %22, ptr @_ZL10g_sink_idx, align 4, !dbg !2618, !tbaa !2062
  %23 = sext i32 %19 to i64, !dbg !2619
  %24 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %23, !dbg !2619
  store ptr %2, ptr %24, align 8, !dbg !2620, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr undef, metadata !2621, metadata !DIExpression()), !dbg !2624
  call void @llvm.dbg.value(metadata ptr %3, metadata !2626, metadata !DIExpression()), !dbg !2632
  call void @llvm.dbg.value(metadata ptr %3, metadata !2636, metadata !DIExpression()), !dbg !2642
  call void @llvm.dbg.value(metadata ptr %15, metadata !2645, metadata !DIExpression()), !dbg !2652
  %25 = atomicrmw add ptr %15, i64 -1 acq_rel, align 8, !dbg !2655
  %26 = icmp eq i64 %25, 0, !dbg !2656
  br i1 %26, label %27, label %31, !dbg !2657

27:                                               ; preds = %14
  %28 = load ptr, ptr %3, align 8, !dbg !2658, !tbaa !2580
  %29 = getelementptr inbounds ptr, ptr %28, i64 2, !dbg !2658
  %30 = load ptr, ptr %29, align 8, !dbg !2658
  tail call void %30(ptr noundef nonnull align 8 dereferenceable(16) %3) #27, !dbg !2658
  tail call void @_ZNSt3__h19__shared_weak_count14__release_weakEv(ptr noundef nonnull align 8 dereferenceable(24) %3) #27, !dbg !2660
  br label %31, !dbg !2660

31:                                               ; preds = %14, %27
  call void @llvm.dbg.value(metadata ptr undef, metadata !2661, metadata !DIExpression()), !dbg !2665
  call void @llvm.dbg.value(metadata ptr undef, metadata !2667, metadata !DIExpression()), !dbg !2672
  call void @llvm.dbg.value(metadata ptr null, metadata !2670, metadata !DIExpression()), !dbg !2672
  call void @llvm.dbg.value(metadata ptr %1, metadata !2671, metadata !DIExpression()), !dbg !2672
  call void @llvm.dbg.value(metadata ptr null, metadata !2518, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2521
  %32 = icmp eq ptr %1, null, !dbg !2675
  br i1 %32, label %34, label %33, !dbg !2677

33:                                               ; preds = %31
  tail call void @free(ptr noundef nonnull %1) #21, !dbg !2678
  br label %34

34:                                               ; preds = %33, %31
  ret void, !dbg !2679

35:                                               ; preds = %8
  call void @llvm.dbg.value(metadata ptr undef, metadata !2661, metadata !DIExpression()), !dbg !2680
  call void @llvm.dbg.value(metadata ptr undef, metadata !2667, metadata !DIExpression()), !dbg !2682
  call void @llvm.dbg.value(metadata ptr null, metadata !2670, metadata !DIExpression()), !dbg !2682
  call void @llvm.dbg.value(metadata ptr %1, metadata !2671, metadata !DIExpression()), !dbg !2682
  call void @llvm.dbg.value(metadata ptr null, metadata !2518, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !2521
  %36 = icmp eq ptr %1, null, !dbg !2684
  br i1 %36, label %38, label %37, !dbg !2685

37:                                               ; preds = %35
  tail call void @free(ptr noundef nonnull %1) #21, !dbg !2686
  br label %38

38:                                               ; preds = %37, %35
  resume { ptr, i32 } %9, !dbg !2679
}

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free")
declare void @free(ptr allocptr nocapture noundef) #7

; Function Attrs: noinline
define void @_Z16test_class_storev() local_unnamed_addr #6 !dbg !2687 {
  %1 = tail call noalias noundef nonnull dereferenceable(16) ptr @_Znwm(i64 noundef 16) #23, !dbg !2693, !memtracer !2694, !heapallocsite !972
  call void @llvm.dbg.value(metadata ptr %1, metadata !2695, metadata !DIExpression()), !dbg !2699
  store ptr getelementptr inbounds ({ [4 x ptr] }, ptr @_ZTV4Base, i64 0, inrange i32 0, i64 2), ptr %1, align 8, !dbg !2701, !tbaa !2580, !memtracer !2702
  call void @llvm.dbg.value(metadata ptr %1, metadata !2689, metadata !DIExpression()), !dbg !2703
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2704, !memtracer !2705
  %3 = getelementptr inbounds %class.Base, ptr %1, i64 0, i32 1, !dbg !2706
  store ptr %2, ptr %3, align 8, !dbg !2707, !tbaa !2708, !memtracer !2705
  %4 = tail call noalias noundef nonnull dereferenceable(24) ptr @_Znwm(i64 noundef 24) #23, !dbg !2710, !memtracer !2711, !heapallocsite !984
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %4, i8 0, i64 16, i1 false), !dbg !2712
  call void @llvm.dbg.value(metadata ptr %4, metadata !2713, metadata !DIExpression()), !dbg !2720
  store ptr getelementptr inbounds ({ [4 x ptr] }, ptr @_ZTV7Derived, i64 0, inrange i32 0, i64 2), ptr %4, align 8, !dbg !2722, !tbaa !2580, !memtracer !2723
  call void @llvm.dbg.value(metadata ptr %4, metadata !2691, metadata !DIExpression()), !dbg !2703
  %5 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2724, !memtracer !2725
  %6 = getelementptr inbounds %class.Derived, ptr %4, i64 0, i32 1, !dbg !2726
  store ptr %5, ptr %6, align 8, !dbg !2727, !tbaa !2728, !memtracer !2725
  call void @llvm.dbg.value(metadata ptr %1, metadata !2056, metadata !DIExpression()), !dbg !2730
  %7 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2732, !tbaa !2062
  %8 = add nsw i32 %7, 1, !dbg !2732
  %9 = sext i32 %7 to i64, !dbg !2733
  %10 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %9, !dbg !2733
  store ptr %1, ptr %10, align 8, !dbg !2734, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %2, metadata !2056, metadata !DIExpression()), !dbg !2735
  %11 = add nsw i32 %7, 2, !dbg !2737
  %12 = sext i32 %8 to i64, !dbg !2738
  %13 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %12, !dbg !2738
  store ptr %2, ptr %13, align 8, !dbg !2739, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %4, metadata !2056, metadata !DIExpression()), !dbg !2740
  %14 = add nsw i32 %7, 3, !dbg !2742
  %15 = sext i32 %11 to i64, !dbg !2743
  %16 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %15, !dbg !2743
  store ptr %4, ptr %16, align 8, !dbg !2744, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %5, metadata !2056, metadata !DIExpression()), !dbg !2745
  %17 = add nsw i32 %7, 4, !dbg !2747
  store i32 %17, ptr @_ZL10g_sink_idx, align 4, !dbg !2747, !tbaa !2062
  %18 = sext i32 %14 to i64, !dbg !2748
  %19 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %18, !dbg !2748
  store ptr %5, ptr %19, align 8, !dbg !2749, !tbaa !2068, !memtracer !2070
  ret void, !dbg !2750
}

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #8

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #9

; Function Attrs: mustprogress noinline nounwind willreturn
define void @_Z15test_move_storev() local_unnamed_addr #10 personality ptr @__gxx_personality_v0 !dbg !2751 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2755
  call void @llvm.dbg.value(metadata ptr %1, metadata !2753, metadata !DIExpression()), !dbg !2756
  call void @llvm.dbg.value(metadata ptr %1, metadata !2754, metadata !DIExpression()), !dbg !2756
  call void @llvm.dbg.value(metadata ptr null, metadata !2753, metadata !DIExpression()), !dbg !2756
  call void @llvm.dbg.value(metadata ptr %1, metadata !2056, metadata !DIExpression()), !dbg !2757
  %2 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2759, !tbaa !2062
  %3 = add nsw i32 %2, 1, !dbg !2759
  store i32 %3, ptr @_ZL10g_sink_idx, align 4, !dbg !2759, !tbaa !2062
  %4 = sext i32 %2 to i64, !dbg !2760
  %5 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %4, !dbg !2760
  store ptr %1, ptr %5, align 8, !dbg !2761, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr undef, metadata !2762, metadata !DIExpression()), !dbg !2766
  tail call void @free(ptr noundef %1) #21, !dbg !2768
  call void @llvm.dbg.value(metadata ptr undef, metadata !2762, metadata !DIExpression()), !dbg !2770
  ret void, !dbg !2772
}

; Function Attrs: mustprogress noinline
define void @_Z24test_multi_inherit_storev() local_unnamed_addr #11 !dbg !2773 {
  %1 = tail call noalias noundef nonnull dereferenceable(24) ptr @_Znwm(i64 noundef 24) #23, !dbg !2788, !memtracer !2789, !heapallocsite !2777
  call void @llvm.dbg.value(metadata ptr %1, metadata !2775, metadata !DIExpression()), !dbg !2790
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2791, !memtracer !2792
  store ptr %2, ptr %1, align 8, !dbg !2793, !tbaa !2794, !memtracer !2792
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2796, !memtracer !2797
  %4 = getelementptr inbounds i8, ptr %1, i64 8, !dbg !2798
  store ptr %3, ptr %4, align 8, !dbg !2799, !tbaa !2800, !memtracer !2797
  %5 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #20, !dbg !2802, !memtracer !2803
  %6 = getelementptr inbounds %class.MultiDerived, ptr %1, i64 0, i32 2, !dbg !2804
  store ptr %5, ptr %6, align 8, !dbg !2805, !tbaa !2806, !memtracer !2803
  call void @llvm.dbg.value(metadata ptr %1, metadata !2056, metadata !DIExpression()), !dbg !2808
  %7 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !2810, !tbaa !2062
  %8 = add nsw i32 %7, 1, !dbg !2810
  %9 = sext i32 %7 to i64, !dbg !2811
  %10 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %9, !dbg !2811
  store ptr %1, ptr %10, align 8, !dbg !2812, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %2, metadata !2056, metadata !DIExpression()), !dbg !2813
  %11 = add nsw i32 %7, 2, !dbg !2815
  %12 = sext i32 %8 to i64, !dbg !2816
  %13 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %12, !dbg !2816
  store ptr %2, ptr %13, align 8, !dbg !2817, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %3, metadata !2056, metadata !DIExpression()), !dbg !2818
  %14 = add nsw i32 %7, 3, !dbg !2820
  %15 = sext i32 %11 to i64, !dbg !2821
  %16 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %15, !dbg !2821
  store ptr %3, ptr %16, align 8, !dbg !2822, !tbaa !2068, !memtracer !2070
  call void @llvm.dbg.value(metadata ptr %5, metadata !2056, metadata !DIExpression()), !dbg !2823
  %17 = add nsw i32 %7, 4, !dbg !2825
  store i32 %17, ptr @_ZL10g_sink_idx, align 4, !dbg !2825, !tbaa !2062
  %18 = sext i32 %14 to i64, !dbg !2826
  %19 = getelementptr inbounds [32 x ptr], ptr @_ZL6g_sink, i64 0, i64 %18, !dbg !2826
  store ptr %5, ptr %19, align 8, !dbg !2827, !tbaa !2068, !memtracer !2070
  ret void, !dbg !2828
}

; Function Attrs: mustprogress norecurse
define noundef i32 @main() local_unnamed_addr #12 !dbg !2829 {
  tail call void @_Z17test_memcpy_storev() #21, !dbg !2830
  %1 = tail call noundef ptr @_Z17test_strcpy_storePKc(ptr noundef nonnull @.str) #21, !dbg !2831
  tail call void @_Z21test_stl_vector_storev() #21, !dbg !2832
  tail call void @_Z20test_smart_ptr_storev() #21, !dbg !2833
  tail call void @_Z16test_class_storev() #21, !dbg !2834
  tail call void @_Z15test_move_storev() #21, !dbg !2835
  tail call void @_Z24test_multi_inherit_storev() #21, !dbg !2836
  ret i32 0, !dbg !2837
}

; Function Attrs: nounwind
define linkonce_odr void @_ZN4BaseD0Ev(ptr noundef nonnull align 8 dereferenceable(16) %0) unnamed_addr #13 comdat align 2 !dbg !2838 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2840, metadata !DIExpression()), !dbg !2841
  tail call void @_ZdlPv(ptr noundef nonnull %0) #24, !dbg !2842
  ret void, !dbg !2842
}

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(ptr noundef) local_unnamed_addr #14

; Function Attrs: nounwind
define linkonce_odr void @_ZN4BaseD2Ev(ptr noundef nonnull align 8 dereferenceable(16) %0) unnamed_addr #13 comdat align 2 !dbg !2843 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2845, metadata !DIExpression()), !dbg !2846
  ret void, !dbg !2847
}

; Function Attrs: inlinehint nounwind
define linkonce_odr void @_ZN7DerivedD0Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #15 comdat align 2 !dbg !2848 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2851, metadata !DIExpression()), !dbg !2852
  tail call void @_ZdlPv(ptr noundef nonnull %0) #24, !dbg !2853
  ret void, !dbg !2853
}

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) local_unnamed_addr #16 comdat {
  %2 = tail call ptr @__cxa_begin_catch(ptr %0) #22
  tail call void @_ZSt9terminatev() #26
  unreachable
}

declare ptr @__cxa_begin_catch(ptr) local_unnamed_addr

declare void @_ZSt9terminatev() local_unnamed_addr

declare void @__cxa_rethrow() local_unnamed_addr

declare void @__cxa_end_catch() local_unnamed_addr

; Function Attrs: nounwind
declare void @_ZNSt3__h19__shared_weak_countD2Ev(ptr noundef nonnull align 8 dereferenceable(24)) unnamed_addr #17

; Function Attrs: inlinehint nounwind
define linkonce_odr void @_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEED0Ev(ptr noundef nonnull align 8 dereferenceable(40) %0) unnamed_addr #15 comdat align 2 !dbg !2854 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2857, metadata !DIExpression()), !dbg !2858
  tail call void @_ZNSt3__h19__shared_weak_countD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %0) #27, !dbg !2859
  tail call void @_ZdlPv(ptr noundef nonnull %0) #24, !dbg !2859
  ret void, !dbg !2859
}

; Function Attrs: mustprogress nounwind
define linkonce_odr void @_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE16__on_zero_sharedEv(ptr noundef nonnull align 8 dereferenceable(40) %0) unnamed_addr #18 comdat align 2 personality ptr @__gxx_personality_v0 !dbg !2860 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2862, metadata !DIExpression()), !dbg !2863
  %2 = getelementptr inbounds %"class.std::__h::__shared_ptr_pointer", ptr %0, i64 0, i32 1, !dbg !2864
  call void @llvm.dbg.value(metadata ptr %2, metadata !2865, metadata !DIExpression()), !dbg !2868
  %3 = getelementptr inbounds %"class.std::__h::__shared_ptr_pointer", ptr %0, i64 0, i32 1, i32 0, i32 0, i32 1, !dbg !2870
  %4 = load ptr, ptr %3, align 8, !dbg !2871, !tbaa !2068
  %5 = load ptr, ptr %2, align 8, !dbg !2872, !tbaa !2068
  invoke void %4(ptr noundef %5) #21
          to label %6 unwind label %7, !dbg !2864

6:                                                ; preds = %1
  call void @llvm.dbg.value(metadata ptr %2, metadata !2865, metadata !DIExpression()), !dbg !2873
  ret void, !dbg !2875

7:                                                ; preds = %1
  %8 = landingpad { ptr, i32 }
          catch ptr null, !dbg !2864
  %9 = extractvalue { ptr, i32 } %8, 0, !dbg !2864
  tail call void @__clang_call_terminate(ptr %9) #26, !dbg !2864
  unreachable, !dbg !2864
}

; Function Attrs: mustprogress nounwind
define linkonce_odr noundef ptr @_ZNKSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE13__get_deleterERKSt9type_info(ptr noundef nonnull align 8 dereferenceable(40) %0, ptr noundef nonnull align 8 dereferenceable(16) %1) unnamed_addr #18 comdat align 2 !dbg !2876 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !2878, metadata !DIExpression()), !dbg !2880
  call void @llvm.dbg.value(metadata ptr %1, metadata !2879, metadata !DIExpression()), !dbg !2880
  call void @llvm.dbg.value(metadata ptr %1, metadata !2881, metadata !DIExpression()), !dbg !2890
  %3 = getelementptr inbounds %"class.std::type_info", ptr %1, i64 0, i32 1, !dbg !2892
  %4 = load ptr, ptr %3, align 8, !dbg !2892, !tbaa !2893
  call void @llvm.dbg.value(metadata ptr %4, metadata !2895, metadata !DIExpression()), !dbg !2919
  call void @llvm.dbg.value(metadata ptr @_ZTSPFvPvE, metadata !2918, metadata !DIExpression()), !dbg !2919
  %5 = icmp eq ptr %4, @_ZTSPFvPvE, !dbg !2921
  %6 = getelementptr inbounds %"class.std::__h::__shared_ptr_pointer", ptr %0, i64 0, i32 1, i32 0, i32 0, i32 1, !dbg !2922
  %7 = select i1 %5, ptr %6, ptr null, !dbg !2922
  ret ptr %7, !dbg !2923
}

; Function Attrs: nounwind
define linkonce_odr void @_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE21__on_zero_shared_weakEv(ptr noundef nonnull align 8 dereferenceable(40) %0) unnamed_addr #13 comdat align 2 personality ptr @__gxx_personality_v0 !dbg !2924 {
  call void @llvm.dbg.declare(metadata ptr undef, metadata !2927, metadata !DIExpression()), !dbg !2933
  call void @llvm.dbg.value(metadata ptr %0, metadata !2926, metadata !DIExpression()), !dbg !2934
  call void @llvm.dbg.value(metadata ptr undef, metadata !2935, metadata !DIExpression()), !dbg !2941
  call void @llvm.dbg.value(metadata ptr %0, metadata !2938, metadata !DIExpression()), !dbg !2941
  call void @llvm.dbg.value(metadata i64 1, metadata !2939, metadata !DIExpression()), !dbg !2941
  call void @llvm.dbg.value(metadata ptr %0, metadata !2399, metadata !DIExpression()), !dbg !2943
  call void @llvm.dbg.value(metadata i64 1, metadata !2404, metadata !DIExpression(DW_OP_constu, 40, DW_OP_mul, DW_OP_stack_value)), !dbg !2943
  call void @llvm.dbg.value(metadata i64 8, metadata !2405, metadata !DIExpression()), !dbg !2943
  call void @llvm.dbg.value(metadata ptr %0, metadata !2410, metadata !DIExpression()), !dbg !2947
  call void @llvm.dbg.value(metadata i64 1, metadata !2415, metadata !DIExpression(DW_OP_constu, 40, DW_OP_mul, DW_OP_stack_value)), !dbg !2947
  call void @llvm.dbg.value(metadata ptr %0, metadata !2420, metadata !DIExpression()), !dbg !2949
  tail call void @_ZdlPv(ptr noundef nonnull %0) #24, !dbg !2951
  ret void, !dbg !2952
}

; Function Attrs: nounwind
declare void @_ZNSt3__h19__shared_weak_count14__release_weakEv(ptr noundef nonnull align 8 dereferenceable(24)) local_unnamed_addr #17

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #19

attributes #0 = { mustprogress nofree noinline nounwind willreturn "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #4 = { argmemonly mustprogress nocallback nofree nounwind willreturn }
attributes #5 = { argmemonly mustprogress nofree nounwind willreturn "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #6 = { noinline "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #7 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #8 = { nobuiltin allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #9 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }
attributes #10 = { mustprogress noinline nounwind willreturn "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #11 = { mustprogress noinline "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #12 = { mustprogress norecurse "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #13 = { nounwind "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #14 = { nobuiltin nounwind "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #15 = { inlinehint nounwind "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #16 = { noinline noreturn nounwind }
attributes #17 = { nounwind "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #18 = { mustprogress nounwind "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #19 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #20 = { allocsize(0) "reference-tracking"="true" }
attributes #21 = { "reference-tracking"="true" }
attributes #22 = { nounwind }
attributes #23 = { builtin allocsize(0) "reference-tracking"="true" }
attributes #24 = { builtin nounwind "reference-tracking"="true" }
attributes #25 = { noreturn }
attributes #26 = { noreturn nounwind }
attributes #27 = { nounwind "reference-tracking"="true" }

!llvm.dbg.cu = !{!10}
!llvm.module.flags = !{!2039, !2040, !2041, !2042, !2043, !2044}
!llvm.ident = !{!2045}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 92, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "memtracer_cpp_features.cpp", directory: "", checksumkind: CSK_MD5, checksum: "4b19b866744f05d33fd66885be76c89e")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 48, elements: !6)
!4 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5)
!5 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_unsigned_char)
!6 = !{!7}
!7 = !DISubrange(count: 6)
!8 = !DIGlobalVariableExpression(var: !9, expr: !DIExpression())
!9 = distinct !DIGlobalVariable(name: "g_sink", linkageName: "_ZL6g_sink", scope: !10, file: !2, line: 14, type: !2036, isLocal: true, isDefinition: true)
!10 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !11, producer: "clang version 15.0.4", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !12, retainedTypes: !29, globals: !1344, imports: !1352, splitDebugInlining: false, nameTableKind: None)
!11 = !DIFile(filename: "memtracer_cpp_features.cpp", directory: "", checksumkind: CSK_MD5, checksum: "4b19b866744f05d33fd66885be76c89e")
!12 = !{!13, !22}
!13 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "float_denorm_style", scope: !15, file: !14, line: 133, baseType: !17, size: 32, elements: !18, identifier: "_ZTSNSt3__h18float_denorm_styleE")
!14 = !DIFile(filename: "limits", directory: "")
!15 = !DINamespace(name: "__h", scope: !16, exportSymbols: true)
!16 = !DINamespace(name: "std", scope: null)
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !{!19, !20, !21}
!19 = !DIEnumerator(name: "denorm_indeterminate", value: -1)
!20 = !DIEnumerator(name: "denorm_absent", value: 0)
!21 = !DIEnumerator(name: "denorm_present", value: 1)
!22 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "float_round_style", scope: !15, file: !14, line: 124, baseType: !17, size: 32, elements: !23, identifier: "_ZTSNSt3__h17float_round_styleE")
!23 = !{!24, !25, !26, !27, !28}
!24 = !DIEnumerator(name: "round_indeterminate", value: -1)
!25 = !DIEnumerator(name: "round_toward_zero", value: 0)
!26 = !DIEnumerator(name: "round_to_nearest", value: 1)
!27 = !DIEnumerator(name: "round_toward_infinity", value: 2)
!28 = !DIEnumerator(name: "round_toward_neg_infinity", value: 3)
!29 = !{!30, !31, !221, !691, !98, !692, !67, !694, !700, !68, !507, !772, !793, !821, !913, !972, !984, !988, !50, !46, !117, !148, !33, !113, !1002, !1024, !415, !403, !411, !1070, !1110, !825, !1143, !1151, !1187, !1210, !1237, !921, !1293, !1241, !1296, !1304}
!30 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!31 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__destroy_vector", scope: !33, file: !32, line: 430, size: 64, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !682, identifier: "_ZTSNSt3__h6vectorIPcNS_9allocatorIS1_EEE16__destroy_vectorE")
!32 = !DIFile(filename: "vector", directory: "")
!33 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "vector<char *, std::__h::allocator<char *> >", scope: !15, file: !32, line: 341, size: 192, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !34, templateParams: !680, identifier: "_ZTSNSt3__h6vectorIPcNS_9allocatorIS1_EEEE")
!34 = !{!35, !111, !112, !208, !212, !218, !222, !225, !231, !232, !237, !246, !250, !255, !258, !261, !265, !268, !271, !275, !276, !280, !286, !291, !292, !293, !299, !304, !305, !306, !307, !308, !309, !310, !313, !314, !317, !318, !319, !320, !325, !328, !329, !330, !333, !336, !337, !338, !342, !346, !349, !353, !354, !357, !360, !363, !366, !369, !372, !373, !374, !375, !378, !379, !382, !383, !384, !387, !388, !389, !390, !391, !394, !399, !626, !629, !632, !635, !638, !639, !642, !645, !648, !649, !650, !654, !657, !661, !666, !667, !668, !669, !670, !671, !672, !675, !678, !679}
!35 = !DIDerivedType(tag: DW_TAG_member, name: "__begin_", scope: !33, file: !32, line: 675, baseType: !36, size: 64)
!36 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !33, file: !32, line: 354, baseType: !37)
!37 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !39, file: !38, line: 233, baseType: !77)
!38 = !DIFile(filename: "allocator_traits.h", directory: "")
!39 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "allocator_traits<std::__h::allocator<char *> >", scope: !15, file: !38, line: 229, size: 8, flags: DIFlagTypePassByValue, elements: !40, templateParams: !109, identifier: "_ZTSNSt3__h16allocator_traitsINS_9allocatorIPcEEEE")
!40 = !{!41, !106}
!41 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h16allocator_traitsINS_9allocatorIPcEEE8allocateB6v15004ERS3_m", scope: !39, file: !38, line: 261, type: !42, scopeLine: 261, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!42 = !DISubroutineType(types: !43)
!43 = !{!37, !44, !104}
!44 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !45, size: 64)
!45 = !DIDerivedType(tag: DW_TAG_typedef, name: "allocator_type", scope: !39, file: !38, line: 231, baseType: !46)
!46 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "allocator<char *>", scope: !15, file: !47, line: 87, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !48, templateParams: !102, identifier: "_ZTSNSt3__h9allocatorIPcEE")
!47 = !DIFile(filename: "allocator.h", directory: "")
!48 = !{!49, !60, !64, !71, !74, !82, !90, !95, !99}
!49 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !46, baseType: !50, extraData: i32 0)
!50 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__non_trivial_if<true, std::__h::allocator<char *> >", scope: !15, file: !47, line: 76, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !51, templateParams: !56, identifier: "_ZTSNSt3__h16__non_trivial_ifILb1ENS_9allocatorIPcEEEE")
!51 = !{!52}
!52 = !DISubprogram(name: "__non_trivial_if", scope: !50, file: !47, line: 78, type: !53, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!53 = !DISubroutineType(types: !54)
!54 = !{null, !55}
!55 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!56 = !{!57, !59}
!57 = !DITemplateValueParameter(name: "_Cond", type: !58, value: i8 1)
!58 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!59 = !DITemplateTypeParameter(name: "_Unique", type: !46)
!60 = !DISubprogram(name: "allocator", scope: !46, file: !47, line: 99, type: !61, scopeLine: 99, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!61 = !DISubroutineType(types: !62)
!62 = !{null, !63}
!63 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!64 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorIPcE8allocateB6v15004Em", scope: !46, file: !47, line: 106, type: !65, scopeLine: 106, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!65 = !DISubroutineType(types: !66)
!66 = !{!67, !63, !68}
!67 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !30, size: 64)
!68 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !69, line: 46, baseType: !70)
!69 = !DIFile(filename: "stddef.h", directory: "", checksumkind: CSK_MD5, checksum: "b76978376d35d5cd171876ac58ac1256")
!70 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!71 = !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h9allocatorIPcE10deallocateB6v15004EPS1_m", scope: !46, file: !47, line: 124, type: !72, scopeLine: 124, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!72 = !DISubroutineType(types: !73)
!73 = !{null, !63, !67, !68}
!74 = !DISubprogram(name: "address", linkageName: "_ZNKSt3__h9allocatorIPcE7addressB6v15004ERS1_", scope: !46, file: !47, line: 145, type: !75, scopeLine: 145, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!75 = !DISubroutineType(types: !76)
!76 = !{!77, !78, !80}
!77 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !46, file: !47, line: 134, baseType: !67)
!78 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !79, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!79 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !46)
!80 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !46, file: !47, line: 136, baseType: !81)
!81 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !30, size: 64)
!82 = !DISubprogram(name: "address", linkageName: "_ZNKSt3__h9allocatorIPcE7addressB6v15004ERKS1_", scope: !46, file: !47, line: 149, type: !83, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!83 = !DISubroutineType(types: !84)
!84 = !{!85, !78, !88}
!85 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_pointer", scope: !46, file: !47, line: 135, baseType: !86)
!86 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !87, size: 64)
!87 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !30)
!88 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !46, file: !47, line: 137, baseType: !89)
!89 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !87, size: 64)
!90 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorIPcE8allocateB6v15004EmPKv", scope: !46, file: !47, line: 154, type: !91, scopeLine: 154, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!91 = !DISubroutineType(types: !92)
!92 = !{!67, !63, !68, !93}
!93 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !94, size: 64)
!94 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!95 = !DISubprogram(name: "max_size", linkageName: "_ZNKSt3__h9allocatorIPcE8max_sizeB6v15004Ev", scope: !46, file: !47, line: 158, type: !96, scopeLine: 158, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!96 = !DISubroutineType(types: !97)
!97 = !{!98, !78}
!98 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_type", file: !47, line: 92, baseType: !68)
!99 = !DISubprogram(name: "destroy", linkageName: "_ZNSt3__h9allocatorIPcE7destroyB6v15004EPS1_", scope: !46, file: !47, line: 169, type: !100, scopeLine: 169, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!100 = !DISubroutineType(types: !101)
!101 = !{null, !63, !77}
!102 = !{!103}
!103 = !DITemplateTypeParameter(name: "_Tp", type: !30)
!104 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_type", scope: !39, file: !38, line: 238, baseType: !105)
!105 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_type", scope: !46, file: !47, line: 92, baseType: !68)
!106 = !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h16allocator_traitsINS_9allocatorIPcEEE10deallocateB6v15004ERS3_PS2_m", scope: !39, file: !38, line: 281, type: !107, scopeLine: 281, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!107 = !DISubroutineType(types: !108)
!108 = !{null, !44, !37, !104}
!109 = !{!110}
!110 = !DITemplateTypeParameter(name: "_Alloc", type: !46)
!111 = !DIDerivedType(tag: DW_TAG_member, name: "__end_", scope: !33, file: !32, line: 676, baseType: !36, size: 64, offset: 64)
!112 = !DIDerivedType(tag: DW_TAG_member, name: "__end_cap_", scope: !33, file: !32, line: 677, baseType: !113, size: 64, offset: 128)
!113 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__compressed_pair<char **, std::__h::allocator<char *> >", scope: !15, file: !114, line: 83, size: 64, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !115, templateParams: !205, identifier: "_ZTSNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEEE")
!114 = !DIFile(filename: "compressed_pair.h", directory: "")
!115 = !{!116, !147, !177, !181, !186, !189, !192, !197, !201}
!116 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !113, baseType: !117, extraData: i32 0)
!117 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<char **, 0, false>", scope: !15, file: !114, line: 30, size: 64, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !118, templateParams: !143, identifier: "_ZTSNSt3__h22__compressed_pair_elemIPPcLi0ELb0EEE")
!118 = !{!119, !120, !126, !130, !135}
!119 = !DIDerivedType(tag: DW_TAG_member, name: "__value_", scope: !117, file: !114, line: 53, baseType: !67, size: 64, flags: DIFlagPrivate)
!120 = !DISubprogram(name: "__compressed_pair_elem", scope: !117, file: !114, line: 35, type: !121, scopeLine: 35, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!121 = !DISubroutineType(types: !122)
!122 = !{null, !123, !124}
!123 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !117, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!124 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__default_init_tag", scope: !15, file: !114, line: 26, size: 8, flags: DIFlagTypePassByValue, elements: !125, identifier: "_ZTSNSt3__h18__default_init_tagE")
!125 = !{}
!126 = !DISubprogram(name: "__compressed_pair_elem", scope: !117, file: !114, line: 36, type: !127, scopeLine: 36, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!127 = !DISubroutineType(types: !128)
!128 = !{null, !123, !129}
!129 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__value_init_tag", scope: !15, file: !114, line: 27, size: 8, flags: DIFlagTypePassByValue, elements: !125, identifier: "_ZTSNSt3__h16__value_init_tagE")
!130 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemIPPcLi0ELb0EE5__getB6v15004Ev", scope: !117, file: !114, line: 49, type: !131, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!131 = !DISubroutineType(types: !132)
!132 = !{!133, !123}
!133 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !117, file: !114, line: 32, baseType: !134)
!134 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !67, size: 64)
!135 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemIPPcLi0ELb0EE5__getB6v15004Ev", scope: !117, file: !114, line: 50, type: !136, scopeLine: 50, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!136 = !DISubroutineType(types: !137)
!137 = !{!138, !141}
!138 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !117, file: !114, line: 33, baseType: !139)
!139 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !140, size: 64)
!140 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !67)
!141 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !142, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!142 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !117)
!143 = !{!144, !145, !146}
!144 = !DITemplateTypeParameter(name: "_Tp", type: !67)
!145 = !DITemplateValueParameter(name: "_Idx", type: !17, value: i32 0)
!146 = !DITemplateValueParameter(name: "_CanBeEmptyBase", type: !58, value: i8 0)
!147 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !113, baseType: !148, extraData: i32 0)
!148 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<std::__h::allocator<char *>, 1, true>", scope: !15, file: !114, line: 57, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !149, templateParams: !173, identifier: "_ZTSNSt3__h22__compressed_pair_elemINS_9allocatorIPcEELi1ELb1EEE")
!149 = !{!150, !151, !155, !158, !161, !166}
!150 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !148, baseType: !46, flags: DIFlagPrivate, extraData: i32 0)
!151 = !DISubprogram(name: "__compressed_pair_elem", scope: !148, file: !114, line: 63, type: !152, scopeLine: 63, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!152 = !DISubroutineType(types: !153)
!153 = !{null, !154}
!154 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !148, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!155 = !DISubprogram(name: "__compressed_pair_elem", scope: !148, file: !114, line: 64, type: !156, scopeLine: 64, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!156 = !DISubroutineType(types: !157)
!157 = !{null, !154, !124}
!158 = !DISubprogram(name: "__compressed_pair_elem", scope: !148, file: !114, line: 65, type: !159, scopeLine: 65, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!159 = !DISubroutineType(types: !160)
!160 = !{null, !154, !129}
!161 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemINS_9allocatorIPcEELi1ELb1EE5__getB6v15004Ev", scope: !148, file: !114, line: 78, type: !162, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!162 = !DISubroutineType(types: !163)
!163 = !{!164, !154}
!164 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !148, file: !114, line: 59, baseType: !165)
!165 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !46, size: 64)
!166 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemINS_9allocatorIPcEELi1ELb1EE5__getB6v15004Ev", scope: !148, file: !114, line: 79, type: !167, scopeLine: 79, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!167 = !DISubroutineType(types: !168)
!168 = !{!169, !171}
!169 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !148, file: !114, line: 60, baseType: !170)
!170 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !79, size: 64)
!171 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !172, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!172 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !148)
!173 = !{!174, !175, !176}
!174 = !DITemplateTypeParameter(name: "_Tp", type: !46)
!175 = !DITemplateValueParameter(name: "_Idx", type: !17, value: i32 1)
!176 = !DITemplateValueParameter(name: "_CanBeEmptyBase", type: !58, value: i8 1)
!177 = !DISubprogram(name: "first", linkageName: "_ZNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE5firstB6v15004Ev", scope: !113, file: !114, line: 120, type: !178, scopeLine: 120, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!178 = !DISubroutineType(types: !179)
!179 = !{!133, !180}
!180 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !113, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!181 = !DISubprogram(name: "first", linkageName: "_ZNKSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE5firstB6v15004Ev", scope: !113, file: !114, line: 125, type: !182, scopeLine: 125, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!182 = !DISubroutineType(types: !183)
!183 = !{!138, !184}
!184 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !185, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!185 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !113)
!186 = !DISubprogram(name: "second", linkageName: "_ZNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE6secondB6v15004Ev", scope: !113, file: !114, line: 130, type: !187, scopeLine: 130, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!187 = !DISubroutineType(types: !188)
!188 = !{!164, !180}
!189 = !DISubprogram(name: "second", linkageName: "_ZNKSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE6secondB6v15004Ev", scope: !113, file: !114, line: 135, type: !190, scopeLine: 135, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!190 = !DISubroutineType(types: !191)
!191 = !{!169, !184}
!192 = !DISubprogram(name: "__get_first_base", linkageName: "_ZNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE16__get_first_baseB6v15004EPS5_", scope: !113, file: !114, line: 140, type: !193, scopeLine: 140, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!193 = !DISubroutineType(types: !194)
!194 = !{!195, !196}
!195 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !117, size: 64)
!196 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !113, size: 64)
!197 = !DISubprogram(name: "__get_second_base", linkageName: "_ZNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE17__get_second_baseB6v15004EPS5_", scope: !113, file: !114, line: 144, type: !198, scopeLine: 144, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!198 = !DISubroutineType(types: !199)
!199 = !{!200, !196}
!200 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !148, size: 64)
!201 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEE4swapB6v15004ERS5_", scope: !113, file: !114, line: 149, type: !202, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!202 = !DISubroutineType(types: !203)
!203 = !{null, !180, !204}
!204 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !113, size: 64)
!205 = !{!206, !207}
!206 = !DITemplateTypeParameter(name: "_T1", type: !67)
!207 = !DITemplateTypeParameter(name: "_T2", type: !46)
!208 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 365, type: !209, scopeLine: 365, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!209 = !DISubroutineType(types: !210)
!210 = !{null, !211}
!211 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!212 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 369, type: !213, scopeLine: 369, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!213 = !DISubroutineType(types: !214)
!214 = !{null, !211, !215}
!215 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !216, size: 64)
!216 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !217)
!217 = !DIDerivedType(tag: DW_TAG_typedef, name: "allocator_type", scope: !33, file: !32, line: 348, baseType: !46)
!218 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 379, type: !219, scopeLine: 379, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!219 = !DISubroutineType(types: !220)
!220 = !{null, !211, !221}
!221 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_type", scope: !33, file: !32, line: 352, baseType: !104)
!222 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 381, type: !223, scopeLine: 381, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!223 = !DISubroutineType(types: !224)
!224 = !{null, !211, !221, !215}
!225 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 383, type: !226, scopeLine: 383, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!226 = !DISubroutineType(types: !227)
!227 = !{null, !211, !221, !228}
!228 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !229, size: 64)
!229 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !230)
!230 = !DIDerivedType(tag: DW_TAG_typedef, name: "value_type", scope: !33, file: !32, line: 347, baseType: !30)
!231 = !DISubprogram(name: "~vector", scope: !33, file: !32, line: 449, type: !209, scopeLine: 449, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!232 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 451, type: !233, scopeLine: 451, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!233 = !DISubroutineType(types: !234)
!234 = !{null, !211, !235}
!235 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !236, size: 64)
!236 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !33)
!237 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 452, type: !238, scopeLine: 452, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!238 = !DISubroutineType(types: !239)
!239 = !{null, !211, !235, !240}
!240 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !241, size: 64)
!241 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !242)
!242 = !DIDerivedType(tag: DW_TAG_typedef, name: "type", scope: !244, file: !243, line: 21, baseType: !46)
!243 = !DIFile(filename: "type_identity.h", directory: "")
!244 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__type_identity<std::__h::allocator<char *> >", scope: !15, file: !243, line: 21, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !245, identifier: "_ZTSNSt3__h15__type_identityINS_9allocatorIPcEEEE")
!245 = !{!174}
!246 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEEaSB6v15004ERKS4_", scope: !33, file: !32, line: 454, type: !247, scopeLine: 454, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!247 = !DISubroutineType(types: !248)
!248 = !{!249, !211, !235}
!249 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !33, size: 64)
!250 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 458, type: !251, scopeLine: 458, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!251 = !DISubroutineType(types: !252)
!252 = !{null, !211, !253}
!253 = !DICompositeType(tag: DW_TAG_class_type, name: "initializer_list<char *>", scope: !16, file: !254, line: 59, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSSt16initializer_listIPcE")
!254 = !DIFile(filename: "initializer_list", directory: "")
!255 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 461, type: !256, scopeLine: 461, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!256 = !DISubroutineType(types: !257)
!257 = !{null, !211, !253, !215}
!258 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEEaSB6v15004ESt16initializer_listIS1_E", scope: !33, file: !32, line: 464, type: !259, scopeLine: 464, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!259 = !DISubroutineType(types: !260)
!260 = !{!249, !211, !253}
!261 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 469, type: !262, scopeLine: 469, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!262 = !DISubroutineType(types: !263)
!263 = !{null, !211, !264}
!264 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !33, size: 64)
!265 = !DISubprogram(name: "vector", scope: !33, file: !32, line: 477, type: !266, scopeLine: 477, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!266 = !DISubroutineType(types: !267)
!267 = !{null, !211, !264, !240}
!268 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEEaSB6v15004EOS4_", scope: !33, file: !32, line: 479, type: !269, scopeLine: 479, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!269 = !DISubroutineType(types: !270)
!270 = !{!249, !211, !264}
!271 = !DISubprogram(name: "assign", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6assignEmRKS1_", scope: !33, file: !32, line: 502, type: !272, scopeLine: 502, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!272 = !DISubroutineType(types: !273)
!273 = !{null, !211, !221, !274}
!274 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !33, file: !32, line: 351, baseType: !228)
!275 = !DISubprogram(name: "assign", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6assignB6v15004ESt16initializer_listIS1_E", scope: !33, file: !32, line: 506, type: !251, scopeLine: 506, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!276 = !DISubprogram(name: "get_allocator", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE13get_allocatorB6v15004Ev", scope: !33, file: !32, line: 511, type: !277, scopeLine: 511, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!277 = !DISubroutineType(types: !278)
!278 = !{!217, !279}
!279 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !236, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!280 = !DISubprogram(name: "begin", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE5beginB6v15004Ev", scope: !33, file: !32, line: 514, type: !281, scopeLine: 514, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!281 = !DISubroutineType(types: !282)
!282 = !{!283, !211}
!283 = !DIDerivedType(tag: DW_TAG_typedef, name: "iterator", scope: !33, file: !32, line: 356, baseType: !284)
!284 = !DICompositeType(tag: DW_TAG_class_type, name: "__wrap_iter<char **>", scope: !15, file: !285, line: 27, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSNSt3__h11__wrap_iterIPPcEE")
!285 = !DIFile(filename: "wrap_iter.h", directory: "")
!286 = !DISubprogram(name: "begin", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE5beginB6v15004Ev", scope: !33, file: !32, line: 515, type: !287, scopeLine: 515, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!287 = !DISubroutineType(types: !288)
!288 = !{!289, !279}
!289 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_iterator", scope: !33, file: !32, line: 357, baseType: !290)
!290 = !DICompositeType(tag: DW_TAG_class_type, name: "__wrap_iter<char *const *>", scope: !15, file: !285, line: 27, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSNSt3__h11__wrap_iterIPKPcEE")
!291 = !DISubprogram(name: "end", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE3endB6v15004Ev", scope: !33, file: !32, line: 516, type: !281, scopeLine: 516, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!292 = !DISubprogram(name: "end", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE3endB6v15004Ev", scope: !33, file: !32, line: 517, type: !287, scopeLine: 517, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!293 = !DISubprogram(name: "rbegin", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6rbeginB6v15004Ev", scope: !33, file: !32, line: 520, type: !294, scopeLine: 520, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!294 = !DISubroutineType(types: !295)
!295 = !{!296, !211}
!296 = !DIDerivedType(tag: DW_TAG_typedef, name: "reverse_iterator", scope: !33, file: !32, line: 358, baseType: !297)
!297 = !DICompositeType(tag: DW_TAG_class_type, name: "reverse_iterator<std::__h::__wrap_iter<char **> >", scope: !15, file: !298, line: 43, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSNSt3__h16reverse_iteratorINS_11__wrap_iterIPPcEEEE")
!298 = !DIFile(filename: "reverse_iterator.h", directory: "")
!299 = !DISubprogram(name: "rbegin", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE6rbeginB6v15004Ev", scope: !33, file: !32, line: 523, type: !300, scopeLine: 523, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!300 = !DISubroutineType(types: !301)
!301 = !{!302, !279}
!302 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reverse_iterator", scope: !33, file: !32, line: 359, baseType: !303)
!303 = !DICompositeType(tag: DW_TAG_class_type, name: "reverse_iterator<std::__h::__wrap_iter<char *const *> >", scope: !15, file: !298, line: 43, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSNSt3__h16reverse_iteratorINS_11__wrap_iterIPKPcEEEE")
!304 = !DISubprogram(name: "rend", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE4rendB6v15004Ev", scope: !33, file: !32, line: 526, type: !294, scopeLine: 526, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!305 = !DISubprogram(name: "rend", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE4rendB6v15004Ev", scope: !33, file: !32, line: 529, type: !300, scopeLine: 529, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!306 = !DISubprogram(name: "cbegin", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE6cbeginB6v15004Ev", scope: !33, file: !32, line: 533, type: !287, scopeLine: 533, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!307 = !DISubprogram(name: "cend", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE4cendB6v15004Ev", scope: !33, file: !32, line: 536, type: !287, scopeLine: 536, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!308 = !DISubprogram(name: "crbegin", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE7crbeginB6v15004Ev", scope: !33, file: !32, line: 539, type: !300, scopeLine: 539, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!309 = !DISubprogram(name: "crend", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE5crendB6v15004Ev", scope: !33, file: !32, line: 542, type: !300, scopeLine: 542, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!310 = !DISubprogram(name: "size", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE4sizeB6v15004Ev", scope: !33, file: !32, line: 546, type: !311, scopeLine: 546, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!311 = !DISubroutineType(types: !312)
!312 = !{!221, !279}
!313 = !DISubprogram(name: "capacity", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE8capacityB6v15004Ev", scope: !33, file: !32, line: 549, type: !311, scopeLine: 549, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!314 = !DISubprogram(name: "empty", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE5emptyB6v15004Ev", scope: !33, file: !32, line: 552, type: !315, scopeLine: 552, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!315 = !DISubroutineType(types: !316)
!316 = !{!58, !279}
!317 = !DISubprogram(name: "max_size", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE8max_sizeEv", scope: !33, file: !32, line: 554, type: !311, scopeLine: 554, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!318 = !DISubprogram(name: "reserve", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE7reserveEm", scope: !33, file: !32, line: 555, type: !219, scopeLine: 555, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!319 = !DISubprogram(name: "shrink_to_fit", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE13shrink_to_fitEv", scope: !33, file: !32, line: 556, type: !209, scopeLine: 556, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!320 = !DISubprogram(name: "operator[]", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEEixB6v15004Em", scope: !33, file: !32, line: 558, type: !321, scopeLine: 558, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!321 = !DISubroutineType(types: !322)
!322 = !{!323, !211, !221}
!323 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !33, file: !32, line: 350, baseType: !324)
!324 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !230, size: 64)
!325 = !DISubprogram(name: "operator[]", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEEixB6v15004Em", scope: !33, file: !32, line: 559, type: !326, scopeLine: 559, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!326 = !DISubroutineType(types: !327)
!327 = !{!274, !279, !221}
!328 = !DISubprogram(name: "at", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE2atEm", scope: !33, file: !32, line: 560, type: !321, scopeLine: 560, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!329 = !DISubprogram(name: "at", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE2atEm", scope: !33, file: !32, line: 561, type: !326, scopeLine: 561, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!330 = !DISubprogram(name: "front", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE5frontB6v15004Ev", scope: !33, file: !32, line: 563, type: !331, scopeLine: 563, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!331 = !DISubroutineType(types: !332)
!332 = !{!323, !211}
!333 = !DISubprogram(name: "front", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE5frontB6v15004Ev", scope: !33, file: !32, line: 568, type: !334, scopeLine: 568, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!334 = !DISubroutineType(types: !335)
!335 = !{!274, !279}
!336 = !DISubprogram(name: "back", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE4backB6v15004Ev", scope: !33, file: !32, line: 573, type: !331, scopeLine: 573, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!337 = !DISubprogram(name: "back", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE4backB6v15004Ev", scope: !33, file: !32, line: 578, type: !334, scopeLine: 578, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!338 = !DISubprogram(name: "data", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE4dataB6v15004Ev", scope: !33, file: !32, line: 585, type: !339, scopeLine: 585, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!339 = !DISubroutineType(types: !340)
!340 = !{!341, !211}
!341 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !230, size: 64)
!342 = !DISubprogram(name: "data", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE4dataB6v15004Ev", scope: !33, file: !32, line: 589, type: !343, scopeLine: 589, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!343 = !DISubroutineType(types: !344)
!344 = !{!345, !279}
!345 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !229, size: 64)
!346 = !DISubprogram(name: "push_back", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE9push_backB6v15004ERKS1_", scope: !33, file: !32, line: 592, type: !347, scopeLine: 592, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!347 = !DISubroutineType(types: !348)
!348 = !{null, !211, !274}
!349 = !DISubprogram(name: "push_back", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE9push_backB6v15004EOS1_", scope: !33, file: !32, line: 594, type: !350, scopeLine: 594, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!350 = !DISubroutineType(types: !351)
!351 = !{null, !211, !352}
!352 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !230, size: 64)
!353 = !DISubprogram(name: "pop_back", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE8pop_backEv", scope: !33, file: !32, line: 605, type: !209, scopeLine: 605, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!354 = !DISubprogram(name: "insert", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6insertENS_11__wrap_iterIPKS1_EERS6_", scope: !33, file: !32, line: 607, type: !355, scopeLine: 607, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!355 = !DISubroutineType(types: !356)
!356 = !{!283, !211, !289, !274}
!357 = !DISubprogram(name: "insert", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6insertENS_11__wrap_iterIPKS1_EEOS1_", scope: !33, file: !32, line: 609, type: !358, scopeLine: 609, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!358 = !DISubroutineType(types: !359)
!359 = !{!283, !211, !289, !352}
!360 = !DISubprogram(name: "insert", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6insertENS_11__wrap_iterIPKS1_EEmRS6_", scope: !33, file: !32, line: 613, type: !361, scopeLine: 613, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!361 = !DISubroutineType(types: !362)
!362 = !{!283, !211, !289, !221, !274}
!363 = !DISubprogram(name: "insert", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6insertB6v15004ENS_11__wrap_iterIPKS1_EESt16initializer_listIS1_E", scope: !33, file: !32, line: 636, type: !364, scopeLine: 636, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!364 = !DISubroutineType(types: !365)
!365 = !{!283, !211, !289, !253}
!366 = !DISubprogram(name: "erase", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE5eraseB6v15004ENS_11__wrap_iterIPKS1_EE", scope: !33, file: !32, line: 640, type: !367, scopeLine: 640, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!367 = !DISubroutineType(types: !368)
!368 = !{!283, !211, !289}
!369 = !DISubprogram(name: "erase", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE5eraseENS_11__wrap_iterIPKS1_EES8_", scope: !33, file: !32, line: 641, type: !370, scopeLine: 641, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!370 = !DISubroutineType(types: !371)
!371 = !{!283, !211, !289, !289}
!372 = !DISubprogram(name: "clear", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE5clearB6v15004Ev", scope: !33, file: !32, line: 644, type: !209, scopeLine: 644, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!373 = !DISubprogram(name: "resize", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6resizeEm", scope: !33, file: !32, line: 652, type: !219, scopeLine: 652, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!374 = !DISubprogram(name: "resize", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE6resizeEmRKS1_", scope: !33, file: !32, line: 653, type: !272, scopeLine: 653, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!375 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE4swapERS4_", scope: !33, file: !32, line: 655, type: !376, scopeLine: 655, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!376 = !DISubroutineType(types: !377)
!377 = !{null, !211, !249}
!378 = !DISubprogram(name: "__invariants", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE12__invariantsEv", scope: !33, file: !32, line: 663, type: !315, scopeLine: 663, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!379 = !DISubprogram(name: "__invalidate_iterators_past", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE27__invalidate_iterators_pastB6v15004EPS1_", scope: !33, file: !32, line: 680, type: !380, scopeLine: 680, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!380 = !DISubroutineType(types: !381)
!381 = !{null, !211, !36}
!382 = !DISubprogram(name: "__vallocate", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE11__vallocateB6v15004Em", scope: !33, file: !32, line: 689, type: !219, scopeLine: 689, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!383 = !DISubprogram(name: "__vdeallocate", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE13__vdeallocateEv", scope: !33, file: !32, line: 699, type: !209, scopeLine: 699, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!384 = !DISubprogram(name: "__recommend", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE11__recommendB6v15004Em", scope: !33, file: !32, line: 700, type: !385, scopeLine: 700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!385 = !DISubroutineType(types: !386)
!386 = !{!221, !279, !221}
!387 = !DISubprogram(name: "__construct_at_end", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE18__construct_at_endEm", scope: !33, file: !32, line: 701, type: !219, scopeLine: 701, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!388 = !DISubprogram(name: "__construct_at_end", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE18__construct_at_endEmRKS1_", scope: !33, file: !32, line: 703, type: !272, scopeLine: 703, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!389 = !DISubprogram(name: "__append", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE8__appendEm", scope: !33, file: !32, line: 712, type: !219, scopeLine: 712, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!390 = !DISubprogram(name: "__append", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE8__appendEmRKS1_", scope: !33, file: !32, line: 713, type: !272, scopeLine: 713, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!391 = !DISubprogram(name: "__make_iter", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE11__make_iterB6v15004EPS1_", scope: !33, file: !32, line: 715, type: !392, scopeLine: 715, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!392 = !DISubroutineType(types: !393)
!393 = !{!283, !211, !36}
!394 = !DISubprogram(name: "__make_iter", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE11__make_iterB6v15004EPKS1_", scope: !33, file: !32, line: 717, type: !395, scopeLine: 717, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!395 = !DISubroutineType(types: !396)
!396 = !{!289, !279, !397}
!397 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_pointer", scope: !33, file: !32, line: 355, baseType: !398)
!398 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_pointer", scope: !39, file: !38, line: 234, baseType: !85)
!399 = !DISubprogram(name: "__swap_out_circular_buffer", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS1_RS3_EE", scope: !33, file: !32, line: 718, type: !400, scopeLine: 718, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!400 = !DISubroutineType(types: !401)
!401 = !{null, !211, !402}
!402 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !403, size: 64)
!403 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__split_buffer<char *, std::__h::allocator<char *> &>", scope: !15, file: !404, line: 38, size: 320, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !405, templateParams: !624, identifier: "_ZTSNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEEE")
!404 = !DIFile(filename: "__split_buffer", directory: "")
!405 = !{!406, !408, !409, !410, !466, !472, !475, !480, !486, !490, !495, !498, !501, !504, !508, !509, !513, !516, !519, !523, !528, !529, !530, !531, !534, !537, !538, !539, !540, !546, !552, !553, !554, !557, !558, !561, !562, !566, !567, !568, !569, !570, !573, !576, !595, !611, !612, !613, !614, !617, !618, !621}
!406 = !DIDerivedType(tag: DW_TAG_member, name: "__first_", scope: !403, file: !404, line: 57, baseType: !407, size: 64)
!407 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !403, file: !404, line: 52, baseType: !37)
!408 = !DIDerivedType(tag: DW_TAG_member, name: "__begin_", scope: !403, file: !404, line: 58, baseType: !407, size: 64, offset: 64)
!409 = !DIDerivedType(tag: DW_TAG_member, name: "__end_", scope: !403, file: !404, line: 59, baseType: !407, size: 64, offset: 128)
!410 = !DIDerivedType(tag: DW_TAG_member, name: "__end_cap_", scope: !403, file: !404, line: 60, baseType: !411, size: 128, offset: 192)
!411 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__compressed_pair<char **, std::__h::allocator<char *> &>", scope: !15, file: !114, line: 83, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !412, templateParams: !464, identifier: "_ZTSNSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEEE")
!412 = !{!413, !414, !437, !441, !446, !449, !452, !456, !460}
!413 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !411, baseType: !117, extraData: i32 0)
!414 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !411, baseType: !415, offset: 64, extraData: i32 0)
!415 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<std::__h::allocator<char *> &, 1, false>", scope: !15, file: !114, line: 30, size: 64, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !416, templateParams: !435, identifier: "_ZTSNSt3__h22__compressed_pair_elemIRNS_9allocatorIPcEELi1ELb0EEE")
!416 = !{!417, !418, !422, !425, !429}
!417 = !DIDerivedType(tag: DW_TAG_member, name: "__value_", scope: !415, file: !114, line: 53, baseType: !165, size: 64, flags: DIFlagPrivate)
!418 = !DISubprogram(name: "__compressed_pair_elem", scope: !415, file: !114, line: 35, type: !419, scopeLine: 35, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!419 = !DISubroutineType(types: !420)
!420 = !{null, !421, !124}
!421 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !415, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!422 = !DISubprogram(name: "__compressed_pair_elem", scope: !415, file: !114, line: 36, type: !423, scopeLine: 36, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!423 = !DISubroutineType(types: !424)
!424 = !{null, !421, !129}
!425 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemIRNS_9allocatorIPcEELi1ELb0EE5__getB6v15004Ev", scope: !415, file: !114, line: 49, type: !426, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!426 = !DISubroutineType(types: !427)
!427 = !{!428, !421}
!428 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !415, file: !114, line: 32, baseType: !165)
!429 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemIRNS_9allocatorIPcEELi1ELb0EE5__getB6v15004Ev", scope: !415, file: !114, line: 50, type: !430, scopeLine: 50, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!430 = !DISubroutineType(types: !431)
!431 = !{!432, !433}
!432 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !415, file: !114, line: 33, baseType: !165)
!433 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !434, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!434 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !415)
!435 = !{!436, !175, !146}
!436 = !DITemplateTypeParameter(name: "_Tp", type: !165)
!437 = !DISubprogram(name: "first", linkageName: "_ZNSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE5firstB6v15004Ev", scope: !411, file: !114, line: 120, type: !438, scopeLine: 120, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!438 = !DISubroutineType(types: !439)
!439 = !{!133, !440}
!440 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !411, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!441 = !DISubprogram(name: "first", linkageName: "_ZNKSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE5firstB6v15004Ev", scope: !411, file: !114, line: 125, type: !442, scopeLine: 125, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!442 = !DISubroutineType(types: !443)
!443 = !{!138, !444}
!444 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !445, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!445 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !411)
!446 = !DISubprogram(name: "second", linkageName: "_ZNSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE6secondB6v15004Ev", scope: !411, file: !114, line: 130, type: !447, scopeLine: 130, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!447 = !DISubroutineType(types: !448)
!448 = !{!428, !440}
!449 = !DISubprogram(name: "second", linkageName: "_ZNKSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE6secondB6v15004Ev", scope: !411, file: !114, line: 135, type: !450, scopeLine: 135, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!450 = !DISubroutineType(types: !451)
!451 = !{!432, !444}
!452 = !DISubprogram(name: "__get_first_base", linkageName: "_ZNSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE16__get_first_baseB6v15004EPS6_", scope: !411, file: !114, line: 140, type: !453, scopeLine: 140, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!453 = !DISubroutineType(types: !454)
!454 = !{!195, !455}
!455 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !411, size: 64)
!456 = !DISubprogram(name: "__get_second_base", linkageName: "_ZNSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE17__get_second_baseB6v15004EPS6_", scope: !411, file: !114, line: 144, type: !457, scopeLine: 144, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!457 = !DISubroutineType(types: !458)
!458 = !{!459, !455}
!459 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !415, size: 64)
!460 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h17__compressed_pairIPPcRNS_9allocatorIS1_EEE4swapB6v15004ERS6_", scope: !411, file: !114, line: 149, type: !461, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!461 = !DISubroutineType(types: !462)
!462 = !{null, !440, !463}
!463 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !411, size: 64)
!464 = !{!206, !465}
!465 = !DITemplateTypeParameter(name: "_T2", type: !165)
!466 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 41, type: !467, scopeLine: 41, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!467 = !DISubroutineType(types: !468)
!468 = !{null, !469, !470}
!469 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !403, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!470 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !471, size: 64)
!471 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !403)
!472 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEEaSERKS5_", scope: !403, file: !404, line: 42, type: !473, scopeLine: 42, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!473 = !DISubroutineType(types: !474)
!474 = !{!402, !469, !470}
!475 = !DISubprogram(name: "__alloc", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE7__allocB6v15004Ev", scope: !403, file: !404, line: 65, type: !476, scopeLine: 65, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!476 = !DISubroutineType(types: !477)
!477 = !{!478, !469}
!478 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !479, size: 64)
!479 = !DIDerivedType(tag: DW_TAG_typedef, name: "__alloc_rr", scope: !403, file: !404, line: 46, baseType: !46)
!480 = !DISubprogram(name: "__alloc", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE7__allocB6v15004Ev", scope: !403, file: !404, line: 66, type: !481, scopeLine: 66, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!481 = !DISubroutineType(types: !482)
!482 = !{!483, !485}
!483 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !484, size: 64)
!484 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !479)
!485 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !471, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!486 = !DISubprogram(name: "__end_cap", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE9__end_capB6v15004Ev", scope: !403, file: !404, line: 67, type: !487, scopeLine: 67, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!487 = !DISubroutineType(types: !488)
!488 = !{!489, !469}
!489 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !407, size: 64)
!490 = !DISubprogram(name: "__end_cap", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE9__end_capB6v15004Ev", scope: !403, file: !404, line: 68, type: !491, scopeLine: 68, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!491 = !DISubroutineType(types: !492)
!492 = !{!493, !485}
!493 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !494, size: 64)
!494 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !407)
!495 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 71, type: !496, scopeLine: 71, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!496 = !DISubroutineType(types: !497)
!497 = !{null, !469}
!498 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 74, type: !499, scopeLine: 74, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!499 = !DISubroutineType(types: !500)
!500 = !{null, !469, !478}
!501 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 76, type: !502, scopeLine: 76, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!502 = !DISubroutineType(types: !503)
!503 = !{null, !469, !483}
!504 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 77, type: !505, scopeLine: 77, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!505 = !DISubroutineType(types: !506)
!506 = !{null, !469, !507, !507, !478}
!507 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_type", scope: !403, file: !404, line: 50, baseType: !104)
!508 = !DISubprogram(name: "~__split_buffer", scope: !403, file: !404, line: 78, type: !496, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!509 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 80, type: !510, scopeLine: 80, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!510 = !DISubroutineType(types: !511)
!511 = !{null, !469, !512}
!512 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !403, size: 64)
!513 = !DISubprogram(name: "__split_buffer", scope: !403, file: !404, line: 82, type: !514, scopeLine: 82, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!514 = !DISubroutineType(types: !515)
!515 = !{null, !469, !512, !483}
!516 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEEaSEOS5_", scope: !403, file: !404, line: 83, type: !517, scopeLine: 83, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!517 = !DISubroutineType(types: !518)
!518 = !{!402, !469, !512}
!519 = !DISubprogram(name: "begin", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE5beginB6v15004Ev", scope: !403, file: !404, line: 88, type: !520, scopeLine: 88, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!520 = !DISubroutineType(types: !521)
!521 = !{!522, !469}
!522 = !DIDerivedType(tag: DW_TAG_typedef, name: "iterator", scope: !403, file: !404, line: 54, baseType: !407)
!523 = !DISubprogram(name: "begin", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE5beginB6v15004Ev", scope: !403, file: !404, line: 89, type: !524, scopeLine: 89, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!524 = !DISubroutineType(types: !525)
!525 = !{!526, !485}
!526 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_iterator", scope: !403, file: !404, line: 55, baseType: !527)
!527 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_pointer", scope: !403, file: !404, line: 53, baseType: !398)
!528 = !DISubprogram(name: "end", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE3endB6v15004Ev", scope: !403, file: !404, line: 90, type: !520, scopeLine: 90, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!529 = !DISubprogram(name: "end", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE3endB6v15004Ev", scope: !403, file: !404, line: 91, type: !524, scopeLine: 91, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!530 = !DISubprogram(name: "clear", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE5clearB6v15004Ev", scope: !403, file: !404, line: 94, type: !496, scopeLine: 94, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!531 = !DISubprogram(name: "size", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE4sizeB6v15004Ev", scope: !403, file: !404, line: 96, type: !532, scopeLine: 96, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!532 = !DISubroutineType(types: !533)
!533 = !{!507, !485}
!534 = !DISubprogram(name: "empty", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE5emptyB6v15004Ev", scope: !403, file: !404, line: 97, type: !535, scopeLine: 97, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!535 = !DISubroutineType(types: !536)
!536 = !{!58, !485}
!537 = !DISubprogram(name: "capacity", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE8capacityB6v15004Ev", scope: !403, file: !404, line: 98, type: !532, scopeLine: 98, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!538 = !DISubprogram(name: "__front_spare", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE13__front_spareB6v15004Ev", scope: !403, file: !404, line: 99, type: !532, scopeLine: 99, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!539 = !DISubprogram(name: "__back_spare", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE12__back_spareB6v15004Ev", scope: !403, file: !404, line: 100, type: !532, scopeLine: 100, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!540 = !DISubprogram(name: "front", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE5frontB6v15004Ev", scope: !403, file: !404, line: 102, type: !541, scopeLine: 102, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!541 = !DISubroutineType(types: !542)
!542 = !{!543, !469}
!543 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !403, file: !404, line: 48, baseType: !544)
!544 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !545, size: 64)
!545 = !DIDerivedType(tag: DW_TAG_typedef, name: "value_type", scope: !403, file: !404, line: 44, baseType: !30)
!546 = !DISubprogram(name: "front", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE5frontB6v15004Ev", scope: !403, file: !404, line: 103, type: !547, scopeLine: 103, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!547 = !DISubroutineType(types: !548)
!548 = !{!549, !485}
!549 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !403, file: !404, line: 49, baseType: !550)
!550 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !551, size: 64)
!551 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !545)
!552 = !DISubprogram(name: "back", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE4backB6v15004Ev", scope: !403, file: !404, line: 104, type: !541, scopeLine: 104, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!553 = !DISubprogram(name: "back", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE4backB6v15004Ev", scope: !403, file: !404, line: 105, type: !547, scopeLine: 105, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!554 = !DISubprogram(name: "reserve", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE7reserveEm", scope: !403, file: !404, line: 107, type: !555, scopeLine: 107, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!555 = !DISubroutineType(types: !556)
!556 = !{null, !469, !507}
!557 = !DISubprogram(name: "shrink_to_fit", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE13shrink_to_fitEv", scope: !403, file: !404, line: 108, type: !496, scopeLine: 108, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!558 = !DISubprogram(name: "push_front", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE10push_frontERKS1_", scope: !403, file: !404, line: 109, type: !559, scopeLine: 109, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!559 = !DISubroutineType(types: !560)
!560 = !{null, !469, !549}
!561 = !DISubprogram(name: "push_back", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE9push_backB6v15004ERKS1_", scope: !403, file: !404, line: 110, type: !559, scopeLine: 110, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!562 = !DISubprogram(name: "push_front", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE10push_frontEOS1_", scope: !403, file: !404, line: 111, type: !563, scopeLine: 111, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!563 = !DISubroutineType(types: !564)
!564 = !{null, !469, !565}
!565 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !545, size: 64)
!566 = !DISubprogram(name: "push_back", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE9push_backEOS1_", scope: !403, file: !404, line: 112, type: !563, scopeLine: 112, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!567 = !DISubprogram(name: "pop_front", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE9pop_frontB6v15004Ev", scope: !403, file: !404, line: 116, type: !496, scopeLine: 116, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!568 = !DISubprogram(name: "pop_back", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE8pop_backB6v15004Ev", scope: !403, file: !404, line: 117, type: !496, scopeLine: 117, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!569 = !DISubprogram(name: "__construct_at_end", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE18__construct_at_endEm", scope: !403, file: !404, line: 119, type: !555, scopeLine: 119, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!570 = !DISubprogram(name: "__construct_at_end", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE18__construct_at_endEmRKS1_", scope: !403, file: !404, line: 120, type: !571, scopeLine: 120, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!571 = !DISubroutineType(types: !572)
!572 = !{null, !469, !507, !549}
!573 = !DISubprogram(name: "__destruct_at_begin", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE19__destruct_at_beginB6v15004EPS1_", scope: !403, file: !404, line: 128, type: !574, scopeLine: 128, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!574 = !DISubroutineType(types: !575)
!575 = !{null, !469, !407}
!576 = !DISubprogram(name: "__destruct_at_begin", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE19__destruct_at_beginEPS1_NS_17integral_constantIbLb0EEE", scope: !403, file: !404, line: 131, type: !577, scopeLine: 131, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!577 = !DISubroutineType(types: !578)
!578 = !{null, !469, !407, !579}
!579 = !DIDerivedType(tag: DW_TAG_typedef, name: "false_type", scope: !15, file: !580, line: 38, baseType: !581)
!580 = !DIFile(filename: "integral_constant.h", directory: "")
!581 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "integral_constant<bool, false>", scope: !15, file: !580, line: 21, size: 8, flags: DIFlagTypePassByValue, elements: !582, templateParams: !592, identifier: "_ZTSNSt3__h17integral_constantIbLb0EEE")
!582 = !{!583, !585, !591}
!583 = !DIDerivedType(tag: DW_TAG_member, name: "value", scope: !581, file: !580, line: 23, baseType: !584, flags: DIFlagStaticMember, extraData: i1 false)
!584 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !58)
!585 = !DISubprogram(name: "operator bool", linkageName: "_ZNKSt3__h17integral_constantIbLb0EEcvbB6v15004Ev", scope: !581, file: !580, line: 27, type: !586, scopeLine: 27, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!586 = !DISubroutineType(types: !587)
!587 = !{!588, !589}
!588 = !DIDerivedType(tag: DW_TAG_typedef, name: "value_type", scope: !581, file: !580, line: 24, baseType: !58)
!589 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !590, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!590 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !581)
!591 = !DISubprogram(name: "operator()", linkageName: "_ZNKSt3__h17integral_constantIbLb0EEclB6v15004Ev", scope: !581, file: !580, line: 30, type: !586, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!592 = !{!593, !594}
!593 = !DITemplateTypeParameter(name: "_Tp", type: !58)
!594 = !DITemplateValueParameter(name: "__v", type: !58, value: i8 0)
!595 = !DISubprogram(name: "__destruct_at_begin", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE19__destruct_at_beginEPS1_NS_17integral_constantIbLb1EEE", scope: !403, file: !404, line: 133, type: !596, scopeLine: 133, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!596 = !DISubroutineType(types: !597)
!597 = !{null, !469, !407, !598}
!598 = !DIDerivedType(tag: DW_TAG_typedef, name: "true_type", scope: !15, file: !580, line: 37, baseType: !599)
!599 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "integral_constant<bool, true>", scope: !15, file: !580, line: 21, size: 8, flags: DIFlagTypePassByValue, elements: !600, templateParams: !609, identifier: "_ZTSNSt3__h17integral_constantIbLb1EEE")
!600 = !{!601, !602, !608}
!601 = !DIDerivedType(tag: DW_TAG_member, name: "value", scope: !599, file: !580, line: 23, baseType: !584, flags: DIFlagStaticMember, extraData: i1 true)
!602 = !DISubprogram(name: "operator bool", linkageName: "_ZNKSt3__h17integral_constantIbLb1EEcvbB6v15004Ev", scope: !599, file: !580, line: 27, type: !603, scopeLine: 27, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!603 = !DISubroutineType(types: !604)
!604 = !{!605, !606}
!605 = !DIDerivedType(tag: DW_TAG_typedef, name: "value_type", scope: !599, file: !580, line: 24, baseType: !58)
!606 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !607, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!607 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !599)
!608 = !DISubprogram(name: "operator()", linkageName: "_ZNKSt3__h17integral_constantIbLb1EEclB6v15004Ev", scope: !599, file: !580, line: 30, type: !603, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!609 = !{!593, !610}
!610 = !DITemplateValueParameter(name: "__v", type: !58, value: i8 1)
!611 = !DISubprogram(name: "__destruct_at_end", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE17__destruct_at_endB6v15004EPS1_", scope: !403, file: !404, line: 136, type: !574, scopeLine: 136, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!612 = !DISubprogram(name: "__destruct_at_end", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE17__destruct_at_endB6v15004EPS1_NS_17integral_constantIbLb0EEE", scope: !403, file: !404, line: 139, type: !577, scopeLine: 139, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!613 = !DISubprogram(name: "__destruct_at_end", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE17__destruct_at_endB6v15004EPS1_NS_17integral_constantIbLb1EEE", scope: !403, file: !404, line: 141, type: !596, scopeLine: 141, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!614 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE4swapERS5_", scope: !403, file: !404, line: 143, type: !615, scopeLine: 143, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!615 = !DISubroutineType(types: !616)
!616 = !{null, !469, !402}
!617 = !DISubprogram(name: "__invariants", linkageName: "_ZNKSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE12__invariantsEv", scope: !403, file: !404, line: 147, type: !535, scopeLine: 147, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!618 = !DISubprogram(name: "__move_assign_alloc", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE19__move_assign_allocB6v15004ERS5_NS_17integral_constantIbLb1EEE", scope: !403, file: !404, line: 151, type: !619, scopeLine: 151, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!619 = !DISubroutineType(types: !620)
!620 = !{null, !469, !402, !598}
!621 = !DISubprogram(name: "__move_assign_alloc", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEE19__move_assign_allocB6v15004ERS5_NS_17integral_constantIbLb0EEE", scope: !403, file: !404, line: 158, type: !622, scopeLine: 158, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!622 = !DISubroutineType(types: !623)
!623 = !{null, !469, !402, !579}
!624 = !{!103, !625}
!625 = !DITemplateTypeParameter(name: "_Allocator", type: !165)
!626 = !DISubprogram(name: "__swap_out_circular_buffer", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS1_RS3_EEPS1_", scope: !33, file: !32, line: 719, type: !627, scopeLine: 719, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!627 = !DISubroutineType(types: !628)
!628 = !{!36, !211, !402, !36}
!629 = !DISubprogram(name: "__move_range", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE12__move_rangeEPS1_S5_S5_", scope: !33, file: !32, line: 720, type: !630, scopeLine: 720, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!630 = !DISubroutineType(types: !631)
!631 = !{null, !211, !36, !36, !36}
!632 = !DISubprogram(name: "__move_assign", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE13__move_assignERS4_NS_17integral_constantIbLb1EEE", scope: !33, file: !32, line: 721, type: !633, scopeLine: 721, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!633 = !DISubroutineType(types: !634)
!634 = !{null, !211, !249, !598}
!635 = !DISubprogram(name: "__move_assign", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE13__move_assignERS4_NS_17integral_constantIbLb0EEE", scope: !33, file: !32, line: 723, type: !636, scopeLine: 723, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!636 = !DISubroutineType(types: !637)
!637 = !{null, !211, !249, !579}
!638 = !DISubprogram(name: "__destruct_at_end", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE17__destruct_at_endB6v15004EPS1_", scope: !33, file: !32, line: 726, type: !380, scopeLine: 726, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!639 = !DISubprogram(name: "__annotate_contiguous_container", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE31__annotate_contiguous_containerB6v15004EPKvS6_S6_S6_", scope: !33, file: !32, line: 759, type: !640, scopeLine: 759, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!640 = !DISubroutineType(types: !641)
!641 = !{null, !279, !93, !93, !93, !93}
!642 = !DISubprogram(name: "__annotate_new", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE14__annotate_newB6v15004Em", scope: !33, file: !32, line: 763, type: !643, scopeLine: 763, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!643 = !DISubroutineType(types: !644)
!644 = !{null, !279, !221}
!645 = !DISubprogram(name: "__annotate_delete", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE17__annotate_deleteB6v15004Ev", scope: !33, file: !32, line: 769, type: !646, scopeLine: 769, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!646 = !DISubroutineType(types: !647)
!647 = !{null, !279}
!648 = !DISubprogram(name: "__annotate_increase", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE19__annotate_increaseB6v15004Em", scope: !33, file: !32, line: 775, type: !643, scopeLine: 775, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!649 = !DISubprogram(name: "__annotate_shrink", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE17__annotate_shrinkB6v15004Em", scope: !33, file: !32, line: 782, type: !643, scopeLine: 782, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!650 = !DISubprogram(name: "__alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE7__allocB6v15004Ev", scope: !33, file: !32, line: 824, type: !651, scopeLine: 824, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!651 = !DISubroutineType(types: !652)
!652 = !{!653, !211}
!653 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !217, size: 64)
!654 = !DISubprogram(name: "__alloc", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE7__allocB6v15004Ev", scope: !33, file: !32, line: 827, type: !655, scopeLine: 827, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!655 = !DISubroutineType(types: !656)
!656 = !{!215, !279}
!657 = !DISubprogram(name: "__end_cap", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE9__end_capB6v15004Ev", scope: !33, file: !32, line: 830, type: !658, scopeLine: 830, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!658 = !DISubroutineType(types: !659)
!659 = !{!660, !211}
!660 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !36, size: 64)
!661 = !DISubprogram(name: "__end_cap", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE9__end_capB6v15004Ev", scope: !33, file: !32, line: 833, type: !662, scopeLine: 833, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!662 = !DISubroutineType(types: !663)
!663 = !{!664, !279}
!664 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !665, size: 64)
!665 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !36)
!666 = !DISubprogram(name: "__clear", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE7__clearB6v15004Ev", scope: !33, file: !32, line: 837, type: !209, scopeLine: 837, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!667 = !DISubprogram(name: "__base_destruct_at_end", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE22__base_destruct_at_endB6v15004EPS1_", scope: !33, file: !32, line: 840, type: !380, scopeLine: 840, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!668 = !DISubprogram(name: "__copy_assign_alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE19__copy_assign_allocB6v15004ERKS4_", scope: !33, file: !32, line: 848, type: !233, scopeLine: 848, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!669 = !DISubprogram(name: "__move_assign_alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE19__move_assign_allocB6v15004ERS4_", scope: !33, file: !32, line: 853, type: !376, scopeLine: 853, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!670 = !DISubprogram(name: "__throw_length_error", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE20__throw_length_errorB6v15004Ev", scope: !33, file: !32, line: 861, type: !646, scopeLine: 861, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!671 = !DISubprogram(name: "__throw_out_of_range", linkageName: "_ZNKSt3__h6vectorIPcNS_9allocatorIS1_EEE20__throw_out_of_rangeB6v15004Ev", scope: !33, file: !32, line: 866, type: !646, scopeLine: 866, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!672 = !DISubprogram(name: "__copy_assign_alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE19__copy_assign_allocB6v15004ERKS4_NS_17integral_constantIbLb1EEE", scope: !33, file: !32, line: 871, type: !673, scopeLine: 871, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!673 = !DISubroutineType(types: !674)
!674 = !{null, !211, !235, !598}
!675 = !DISubprogram(name: "__copy_assign_alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE19__copy_assign_allocB6v15004ERKS4_NS_17integral_constantIbLb0EEE", scope: !33, file: !32, line: 883, type: !676, scopeLine: 883, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!676 = !DISubroutineType(types: !677)
!677 = !{null, !211, !235, !579}
!678 = !DISubprogram(name: "__move_assign_alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE19__move_assign_allocB6v15004ERS4_NS_17integral_constantIbLb1EEE", scope: !33, file: !32, line: 887, type: !633, scopeLine: 887, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!679 = !DISubprogram(name: "__move_assign_alloc", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE19__move_assign_allocB6v15004ERS4_NS_17integral_constantIbLb0EEE", scope: !33, file: !32, line: 894, type: !636, scopeLine: 894, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!680 = !{!103, !681}
!681 = !DITemplateTypeParameter(name: "_Allocator", type: !46)
!682 = !{!683, !684, !688}
!683 = !DIDerivedType(tag: DW_TAG_member, name: "__vec_", scope: !31, file: !32, line: 445, baseType: !249, size: 64)
!684 = !DISubprogram(name: "__destroy_vector", scope: !31, file: !32, line: 432, type: !685, scopeLine: 432, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!685 = !DISubroutineType(types: !686)
!686 = !{null, !687, !249}
!687 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!688 = !DISubprogram(name: "operator()", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE16__destroy_vectorclB6v15004Ev", scope: !31, file: !32, line: 434, type: !689, scopeLine: 434, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!689 = !DISubroutineType(types: !690)
!690 = !{null, !687}
!691 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!692 = !DICompositeType(tag: DW_TAG_class_type, name: "length_error", scope: !16, file: !693, line: 149, size: 128, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSSt12length_error")
!693 = !DIFile(filename: "stdexcept", directory: "")
!694 = !DIDerivedType(tag: DW_TAG_typedef, name: "_RevIter", scope: !695, file: !32, line: 923, baseType: !700)
!695 = distinct !DISubprogram(name: "__swap_out_circular_buffer", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE26__swap_out_circular_bufferERNS_14__split_bufferIS1_RS3_EE", scope: !33, file: !32, line: 920, type: !400, scopeLine: 921, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !399, retainedNodes: !696)
!696 = !{!697, !699}
!697 = !DILocalVariable(name: "this", arg: 1, scope: !695, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!698 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!699 = !DILocalVariable(name: "__v", arg: 2, scope: !695, file: !32, line: 718, type: !402)
!700 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "reverse_iterator<char **>", scope: !15, file: !298, line: 43, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !701, templateParams: !742, identifier: "_ZTSNSt3__h16reverse_iteratorIPPcEE")
!701 = !{!702, !722, !723, !724, !728, !731, !736, !744, !749, !753, !756, !757, !758, !764, !767, !768, !769}
!702 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !700, baseType: !703, flags: DIFlagPublic, extraData: i32 0)
!703 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "iterator<std::__h::random_access_iterator_tag, char *, long, char **, char *&>", scope: !15, file: !704, line: 24, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !705, identifier: "_ZTSNSt3__h8iteratorINS_26random_access_iterator_tagEPclPS2_RS2_EE")
!704 = !DIFile(filename: "iterator.h", directory: "")
!705 = !{!706, !103, !718, !720, !721}
!706 = !DITemplateTypeParameter(name: "_Category", type: !707)
!707 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "random_access_iterator_tag", scope: !15, file: !708, line: 54, size: 8, flags: DIFlagTypePassByValue, elements: !709, identifier: "_ZTSNSt3__h26random_access_iterator_tagE")
!708 = !DIFile(filename: "iterator_traits.h", directory: "")
!709 = !{!710}
!710 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !707, baseType: !711, extraData: i32 0)
!711 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bidirectional_iterator_tag", scope: !15, file: !708, line: 53, size: 8, flags: DIFlagTypePassByValue, elements: !712, identifier: "_ZTSNSt3__h26bidirectional_iterator_tagE")
!712 = !{!713}
!713 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !711, baseType: !714, extraData: i32 0)
!714 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "forward_iterator_tag", scope: !15, file: !708, line: 52, size: 8, flags: DIFlagTypePassByValue, elements: !715, identifier: "_ZTSNSt3__h20forward_iterator_tagE")
!715 = !{!716}
!716 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !714, baseType: !717, extraData: i32 0)
!717 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "input_iterator_tag", scope: !15, file: !708, line: 50, size: 8, flags: DIFlagTypePassByValue, elements: !125, identifier: "_ZTSNSt3__h18input_iterator_tagE")
!718 = !DITemplateTypeParameter(name: "_Distance", type: !719)
!719 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!720 = !DITemplateTypeParameter(name: "_Pointer", type: !67)
!721 = !DITemplateTypeParameter(name: "_Reference", type: !81)
!722 = !DIDerivedType(tag: DW_TAG_member, name: "__t", scope: !700, file: !298, line: 55, baseType: !67, size: 64)
!723 = !DIDerivedType(tag: DW_TAG_member, name: "current", scope: !700, file: !298, line: 64, baseType: !67, size: 64, offset: 64, flags: DIFlagProtected)
!724 = !DISubprogram(name: "reverse_iterator", scope: !700, file: !298, line: 85, type: !725, scopeLine: 85, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!725 = !DISubroutineType(types: !726)
!726 = !{null, !727}
!727 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !700, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!728 = !DISubprogram(name: "reverse_iterator", scope: !700, file: !298, line: 88, type: !729, scopeLine: 88, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!729 = !DISubroutineType(types: !730)
!730 = !{null, !727, !67}
!731 = !DISubprogram(name: "base", linkageName: "_ZNKSt3__h16reverse_iteratorIPPcE4baseB6v15004Ev", scope: !700, file: !298, line: 135, type: !732, scopeLine: 135, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!732 = !DISubroutineType(types: !733)
!733 = !{!67, !734}
!734 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !735, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!735 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !700)
!736 = !DISubprogram(name: "operator*", linkageName: "_ZNKSt3__h16reverse_iteratorIPPcEdeB6v15004Ev", scope: !700, file: !298, line: 137, type: !737, scopeLine: 137, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!737 = !DISubroutineType(types: !738)
!738 = !{!739, !734}
!739 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !700, file: !298, line: 80, baseType: !740)
!740 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !741, file: !708, line: 412, baseType: !81)
!741 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "iterator_traits<char **>", scope: !15, file: !708, line: 407, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !742, identifier: "_ZTSNSt3__h15iterator_traitsIPPcEE")
!742 = !{!743}
!743 = !DITemplateTypeParameter(name: "_Iter", type: !67)
!744 = !DISubprogram(name: "operator->", linkageName: "_ZNKSt3__h16reverse_iteratorIPPcEptB6v15004Ev", scope: !700, file: !298, line: 152, type: !745, scopeLine: 152, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!745 = !DISubroutineType(types: !746)
!746 = !{!747, !734}
!747 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !700, file: !298, line: 71, baseType: !748)
!748 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !741, file: !708, line: 411, baseType: !67)
!749 = !DISubprogram(name: "operator++", linkageName: "_ZNSt3__h16reverse_iteratorIPPcEppB6v15004Ev", scope: !700, file: !298, line: 158, type: !750, scopeLine: 158, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!750 = !DISubroutineType(types: !751)
!751 = !{!752, !727}
!752 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !700, size: 64)
!753 = !DISubprogram(name: "operator++", linkageName: "_ZNSt3__h16reverse_iteratorIPPcEppB6v15004Ei", scope: !700, file: !298, line: 160, type: !754, scopeLine: 160, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!754 = !DISubroutineType(types: !755)
!755 = !{!700, !727, !17}
!756 = !DISubprogram(name: "operator--", linkageName: "_ZNSt3__h16reverse_iteratorIPPcEmmB6v15004Ev", scope: !700, file: !298, line: 162, type: !750, scopeLine: 162, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!757 = !DISubprogram(name: "operator--", linkageName: "_ZNSt3__h16reverse_iteratorIPPcEmmB6v15004Ei", scope: !700, file: !298, line: 164, type: !754, scopeLine: 164, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!758 = !DISubprogram(name: "operator+", linkageName: "_ZNKSt3__h16reverse_iteratorIPPcEplB6v15004El", scope: !700, file: !298, line: 166, type: !759, scopeLine: 166, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!759 = !DISubroutineType(types: !760)
!760 = !{!700, !734, !761}
!761 = !DIDerivedType(tag: DW_TAG_typedef, name: "difference_type", scope: !700, file: !298, line: 79, baseType: !762)
!762 = !DIDerivedType(tag: DW_TAG_typedef, name: "difference_type", scope: !741, file: !708, line: 409, baseType: !763)
!763 = !DIDerivedType(tag: DW_TAG_typedef, name: "ptrdiff_t", file: !69, line: 35, baseType: !719)
!764 = !DISubprogram(name: "operator+=", linkageName: "_ZNSt3__h16reverse_iteratorIPPcEpLB6v15004El", scope: !700, file: !298, line: 168, type: !765, scopeLine: 168, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!765 = !DISubroutineType(types: !766)
!766 = !{!752, !727, !761}
!767 = !DISubprogram(name: "operator-", linkageName: "_ZNKSt3__h16reverse_iteratorIPPcEmiB6v15004El", scope: !700, file: !298, line: 170, type: !759, scopeLine: 170, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!768 = !DISubprogram(name: "operator-=", linkageName: "_ZNSt3__h16reverse_iteratorIPPcEmIB6v15004El", scope: !700, file: !298, line: 172, type: !765, scopeLine: 172, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!769 = !DISubprogram(name: "operator[]", linkageName: "_ZNKSt3__h16reverse_iteratorIPPcEixB6v15004El", scope: !700, file: !298, line: 174, type: !770, scopeLine: 174, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!770 = !DISubroutineType(types: !771)
!771 = !{!739, !734, !761}
!772 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<char *, 0, false>", scope: !15, file: !114, line: 30, size: 64, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !773, templateParams: !792, identifier: "_ZTSNSt3__h22__compressed_pair_elemIPcLi0ELb0EEE")
!773 = !{!774, !775, !779, !782, !786}
!774 = !DIDerivedType(tag: DW_TAG_member, name: "__value_", scope: !772, file: !114, line: 53, baseType: !30, size: 64, flags: DIFlagPrivate)
!775 = !DISubprogram(name: "__compressed_pair_elem", scope: !772, file: !114, line: 35, type: !776, scopeLine: 35, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!776 = !DISubroutineType(types: !777)
!777 = !{null, !778, !124}
!778 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !772, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!779 = !DISubprogram(name: "__compressed_pair_elem", scope: !772, file: !114, line: 36, type: !780, scopeLine: 36, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!780 = !DISubroutineType(types: !781)
!781 = !{null, !778, !129}
!782 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemIPcLi0ELb0EE5__getB6v15004Ev", scope: !772, file: !114, line: 49, type: !783, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!783 = !DISubroutineType(types: !784)
!784 = !{!785, !778}
!785 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !772, file: !114, line: 32, baseType: !81)
!786 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemIPcLi0ELb0EE5__getB6v15004Ev", scope: !772, file: !114, line: 50, type: !787, scopeLine: 50, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!787 = !DISubroutineType(types: !788)
!788 = !{!789, !790}
!789 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !772, file: !114, line: 33, baseType: !89)
!790 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !791, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!791 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !772)
!792 = !{!103, !145, !146}
!793 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<void (*)(void *), 1, false>", scope: !15, file: !114, line: 30, size: 64, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !794, templateParams: !819, identifier: "_ZTSNSt3__h22__compressed_pair_elemIPFvPvELi1ELb0EEE")
!794 = !{!795, !799, !803, !806, !811}
!795 = !DIDerivedType(tag: DW_TAG_member, name: "__value_", scope: !793, file: !114, line: 53, baseType: !796, size: 64, flags: DIFlagPrivate)
!796 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !797, size: 64)
!797 = !DISubroutineType(types: !798)
!798 = !{null, !691}
!799 = !DISubprogram(name: "__compressed_pair_elem", scope: !793, file: !114, line: 35, type: !800, scopeLine: 35, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!800 = !DISubroutineType(types: !801)
!801 = !{null, !802, !124}
!802 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !793, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!803 = !DISubprogram(name: "__compressed_pair_elem", scope: !793, file: !114, line: 36, type: !804, scopeLine: 36, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!804 = !DISubroutineType(types: !805)
!805 = !{null, !802, !129}
!806 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemIPFvPvELi1ELb0EE5__getB6v15004Ev", scope: !793, file: !114, line: 49, type: !807, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!807 = !DISubroutineType(types: !808)
!808 = !{!809, !802}
!809 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !793, file: !114, line: 32, baseType: !810)
!810 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !796, size: 64)
!811 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemIPFvPvELi1ELb0EE5__getB6v15004Ev", scope: !793, file: !114, line: 50, type: !812, scopeLine: 50, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!812 = !DISubroutineType(types: !813)
!813 = !{!814, !817}
!814 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !793, file: !114, line: 33, baseType: !815)
!815 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !816, size: 64)
!816 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !796)
!817 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !818, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!818 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !793)
!819 = !{!820, !175, !146}
!820 = !DITemplateTypeParameter(name: "_Tp", type: !796)
!821 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "unique_ptr<char, void (*)(void *)>", scope: !15, file: !822, line: 109, size: 128, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !823, templateParams: !910, identifier: "_ZTSNSt3__h10unique_ptrIcPFvPvEEE")
!822 = !DIFile(filename: "unique_ptr.h", directory: "")
!823 = !{!824, !860, !865, !869, !872, !878, !884, !887, !888, !893, !898, !901, !904, !907}
!824 = !DIDerivedType(tag: DW_TAG_member, name: "__ptr_", scope: !821, file: !822, line: 119, baseType: !825, size: 128)
!825 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__compressed_pair<char *, void (*)(void *)>", scope: !15, file: !114, line: 83, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !826, templateParams: !857, identifier: "_ZTSNSt3__h17__compressed_pairIPcPFvPvEEE")
!826 = !{!827, !828, !829, !833, !838, !841, !844, !849, !853}
!827 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !825, baseType: !772, extraData: i32 0)
!828 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !825, baseType: !793, offset: 64, extraData: i32 0)
!829 = !DISubprogram(name: "first", linkageName: "_ZNSt3__h17__compressed_pairIPcPFvPvEE5firstB6v15004Ev", scope: !825, file: !114, line: 120, type: !830, scopeLine: 120, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!830 = !DISubroutineType(types: !831)
!831 = !{!785, !832}
!832 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !825, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!833 = !DISubprogram(name: "first", linkageName: "_ZNKSt3__h17__compressed_pairIPcPFvPvEE5firstB6v15004Ev", scope: !825, file: !114, line: 125, type: !834, scopeLine: 125, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!834 = !DISubroutineType(types: !835)
!835 = !{!789, !836}
!836 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !837, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!837 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !825)
!838 = !DISubprogram(name: "second", linkageName: "_ZNSt3__h17__compressed_pairIPcPFvPvEE6secondB6v15004Ev", scope: !825, file: !114, line: 130, type: !839, scopeLine: 130, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!839 = !DISubroutineType(types: !840)
!840 = !{!809, !832}
!841 = !DISubprogram(name: "second", linkageName: "_ZNKSt3__h17__compressed_pairIPcPFvPvEE6secondB6v15004Ev", scope: !825, file: !114, line: 135, type: !842, scopeLine: 135, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!842 = !DISubroutineType(types: !843)
!843 = !{!814, !836}
!844 = !DISubprogram(name: "__get_first_base", linkageName: "_ZNSt3__h17__compressed_pairIPcPFvPvEE16__get_first_baseB6v15004EPS5_", scope: !825, file: !114, line: 140, type: !845, scopeLine: 140, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!845 = !DISubroutineType(types: !846)
!846 = !{!847, !848}
!847 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !772, size: 64)
!848 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !825, size: 64)
!849 = !DISubprogram(name: "__get_second_base", linkageName: "_ZNSt3__h17__compressed_pairIPcPFvPvEE17__get_second_baseB6v15004EPS5_", scope: !825, file: !114, line: 144, type: !850, scopeLine: 144, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!850 = !DISubroutineType(types: !851)
!851 = !{!852, !848}
!852 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !793, size: 64)
!853 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h17__compressed_pairIPcPFvPvEE4swapB6v15004ERS5_", scope: !825, file: !114, line: 149, type: !854, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!854 = !DISubroutineType(types: !855)
!855 = !{null, !832, !856}
!856 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !825, size: 64)
!857 = !{!858, !859}
!858 = !DITemplateTypeParameter(name: "_T1", type: !30)
!859 = !DITemplateTypeParameter(name: "_T2", type: !796)
!860 = !DISubprogram(name: "unique_ptr", scope: !821, file: !822, line: 201, type: !861, scopeLine: 201, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!861 = !DISubroutineType(types: !862)
!862 = !{null, !863, !864}
!863 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !821, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!864 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !821, size: 64)
!865 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEEaSB6v15004EOS4_", scope: !821, file: !822, line: 224, type: !866, scopeLine: 224, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!866 = !DISubroutineType(types: !867)
!867 = !{!868, !863, !864}
!868 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !821, size: 64)
!869 = !DISubprogram(name: "~unique_ptr", scope: !821, file: !822, line: 259, type: !870, scopeLine: 259, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!870 = !DISubroutineType(types: !871)
!871 = !{null, !863}
!872 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEEaSB6v15004EDn", scope: !821, file: !822, line: 262, type: !873, scopeLine: 262, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!873 = !DISubroutineType(types: !874)
!874 = !{!868, !863, !875}
!875 = !DIDerivedType(tag: DW_TAG_typedef, name: "nullptr_t", file: !876, line: 48, baseType: !877)
!876 = !DIFile(filename: "stddef.h", directory: "")
!877 = !DIBasicType(tag: DW_TAG_unspecified_type, name: "decltype(nullptr)")
!878 = !DISubprogram(name: "operator*", linkageName: "_ZNKSt3__h10unique_ptrIcPFvPvEEdeB6v15004Ev", scope: !821, file: !822, line: 269, type: !879, scopeLine: 269, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!879 = !DISubroutineType(types: !880)
!880 = !{!881, !882}
!881 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !5, size: 64)
!882 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !883, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!883 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !821)
!884 = !DISubprogram(name: "operator->", linkageName: "_ZNKSt3__h10unique_ptrIcPFvPvEEptB6v15004Ev", scope: !821, file: !822, line: 273, type: !885, scopeLine: 273, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!885 = !DISubroutineType(types: !886)
!886 = !{!30, !882}
!887 = !DISubprogram(name: "get", linkageName: "_ZNKSt3__h10unique_ptrIcPFvPvEE3getB6v15004Ev", scope: !821, file: !822, line: 277, type: !885, scopeLine: 277, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!888 = !DISubprogram(name: "get_deleter", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEE11get_deleterB6v15004Ev", scope: !821, file: !822, line: 281, type: !889, scopeLine: 281, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!889 = !DISubroutineType(types: !890)
!890 = !{!891, !863}
!891 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !892, size: 64)
!892 = !DIDerivedType(tag: DW_TAG_typedef, name: "deleter_type", scope: !821, file: !822, line: 112, baseType: !796)
!893 = !DISubprogram(name: "get_deleter", linkageName: "_ZNKSt3__h10unique_ptrIcPFvPvEE11get_deleterB6v15004Ev", scope: !821, file: !822, line: 285, type: !894, scopeLine: 285, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!894 = !DISubroutineType(types: !895)
!895 = !{!896, !882}
!896 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !897, size: 64)
!897 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !892)
!898 = !DISubprogram(name: "operator bool", linkageName: "_ZNKSt3__h10unique_ptrIcPFvPvEEcvbB6v15004Ev", scope: !821, file: !822, line: 289, type: !899, scopeLine: 289, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!899 = !DISubroutineType(types: !900)
!900 = !{!58, !882}
!901 = !DISubprogram(name: "release", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEE7releaseB6v15004Ev", scope: !821, file: !822, line: 294, type: !902, scopeLine: 294, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!902 = !DISubroutineType(types: !903)
!903 = !{!30, !863}
!904 = !DISubprogram(name: "reset", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEE5resetB6v15004EPc", scope: !821, file: !822, line: 301, type: !905, scopeLine: 301, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!905 = !DISubroutineType(types: !906)
!906 = !{null, !863, !30}
!907 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEE4swapB6v15004ERS4_", scope: !821, file: !822, line: 309, type: !908, scopeLine: 309, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!908 = !DISubroutineType(types: !909)
!909 = !{null, !863, !868}
!910 = !{!911, !912}
!911 = !DITemplateTypeParameter(name: "_Tp", type: !5)
!912 = !DITemplateTypeParameter(name: "_Dp", type: !796)
!913 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "shared_ptr<char>", scope: !15, file: !914, line: 418, size: 128, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !915, templateParams: !971, identifier: "_ZTSNSt3__h10shared_ptrIcEE")
!914 = !DIFile(filename: "shared_ptr.h", directory: "")
!915 = !{!916, !919, !922, !926, !929, !934, !938, !939, !943, !946, !949, !950, !954, !957, !958, !961, !964, !965, !968}
!916 = !DIDerivedType(tag: DW_TAG_member, name: "__ptr_", scope: !913, file: !914, line: 429, baseType: !917, size: 64)
!917 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !918, size: 64)
!918 = !DIDerivedType(tag: DW_TAG_typedef, name: "element_type", scope: !913, file: !914, line: 425, baseType: !5)
!919 = !DIDerivedType(tag: DW_TAG_member, name: "__cntrl_", scope: !913, file: !914, line: 430, baseType: !920, size: 64, offset: 64)
!920 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !921, size: 64)
!921 = !DICompositeType(tag: DW_TAG_class_type, name: "__shared_weak_count", scope: !15, file: !914, line: 186, size: 192, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSNSt3__h19__shared_weak_countE")
!922 = !DISubprogram(name: "shared_ptr", scope: !913, file: !914, line: 434, type: !923, scopeLine: 434, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!923 = !DISubroutineType(types: !924)
!924 = !{null, !925}
!925 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !913, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!926 = !DISubprogram(name: "shared_ptr", scope: !913, file: !914, line: 440, type: !927, scopeLine: 440, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!927 = !DISubroutineType(types: !928)
!928 = !{null, !925, !875}
!929 = !DISubprogram(name: "shared_ptr", scope: !913, file: !914, line: 592, type: !930, scopeLine: 592, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!930 = !DISubroutineType(types: !931)
!931 = !{null, !925, !932}
!932 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !933, size: 64)
!933 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !913)
!934 = !DISubprogram(name: "shared_ptr", scope: !913, file: !914, line: 611, type: !935, scopeLine: 611, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!935 = !DISubroutineType(types: !936)
!936 = !{null, !925, !937}
!937 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !913, size: 64)
!938 = !DISubprogram(name: "~shared_ptr", scope: !913, file: !914, line: 699, type: !923, scopeLine: 699, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!939 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h10shared_ptrIcEaSB6v15004ERKS1_", scope: !913, file: !914, line: 706, type: !940, scopeLine: 706, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!940 = !DISubroutineType(types: !941)
!941 = !{!942, !925, !932}
!942 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !913, size: 64)
!943 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h10shared_ptrIcEaSB6v15004EOS1_", scope: !913, file: !914, line: 721, type: !944, scopeLine: 721, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!944 = !DISubroutineType(types: !945)
!945 = !{!942, !925, !937}
!946 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h10shared_ptrIcE4swapB6v15004ERS1_", scope: !913, file: !914, line: 759, type: !947, scopeLine: 759, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!947 = !DISubroutineType(types: !948)
!948 = !{null, !925, !942}
!949 = !DISubprogram(name: "reset", linkageName: "_ZNSt3__h10shared_ptrIcE5resetB6v15004Ev", scope: !913, file: !914, line: 766, type: !923, scopeLine: 766, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!950 = !DISubprogram(name: "get", linkageName: "_ZNKSt3__h10shared_ptrIcE3getB6v15004Ev", scope: !913, file: !914, line: 799, type: !951, scopeLine: 799, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!951 = !DISubroutineType(types: !952)
!952 = !{!917, !953}
!953 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !933, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!954 = !DISubprogram(name: "operator*", linkageName: "_ZNKSt3__h10shared_ptrIcEdeB6v15004Ev", scope: !913, file: !914, line: 805, type: !955, scopeLine: 805, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!955 = !DISubroutineType(types: !956)
!956 = !{!881, !953}
!957 = !DISubprogram(name: "operator->", linkageName: "_ZNKSt3__h10shared_ptrIcEptB6v15004Ev", scope: !913, file: !914, line: 811, type: !951, scopeLine: 811, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!958 = !DISubprogram(name: "use_count", linkageName: "_ZNKSt3__h10shared_ptrIcE9use_countB6v15004Ev", scope: !913, file: !914, line: 819, type: !959, scopeLine: 819, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!959 = !DISubroutineType(types: !960)
!960 = !{!719, !953}
!961 = !DISubprogram(name: "unique", linkageName: "_ZNKSt3__h10shared_ptrIcE6uniqueB6v15004Ev", scope: !913, file: !914, line: 825, type: !962, scopeLine: 825, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!962 = !DISubroutineType(types: !963)
!963 = !{!58, !953}
!964 = !DISubprogram(name: "operator bool", linkageName: "_ZNKSt3__h10shared_ptrIcEcvbB6v15004Ev", scope: !913, file: !914, line: 831, type: !962, scopeLine: 831, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!965 = !DISubprogram(name: "__owner_equivalent", linkageName: "_ZNKSt3__h10shared_ptrIcE18__owner_equivalentB6v15004ERKS1_", scope: !913, file: !914, line: 851, type: !966, scopeLine: 851, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!966 = !DISubroutineType(types: !967)
!967 = !{!58, !953, !932}
!968 = !DISubprogram(name: "__enable_weak_this", linkageName: "_ZNSt3__h10shared_ptrIcE18__enable_weak_thisB6v15004Ez", scope: !913, file: !914, line: 915, type: !969, scopeLine: 915, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!969 = !DISubroutineType(types: !970)
!970 = !{null, !925, null}
!971 = !{!911}
!972 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Base", file: !2, line: 50, size: 128, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !973, vtableHolder: !972, identifier: "_ZTS4Base")
!973 = !{!974, !979, !980}
!974 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$Base", scope: !2, file: !2, baseType: !975, size: 64, flags: DIFlagArtificial)
!975 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !976, size: 64)
!976 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !977, size: 64)
!977 = !DISubroutineType(types: !978)
!978 = !{!17}
!979 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !972, file: !2, line: 50, baseType: !30, size: 64, offset: 64, flags: DIFlagPublic)
!980 = !DISubprogram(name: "~Base", scope: !972, file: !2, line: 50, type: !981, scopeLine: 50, containingType: !972, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual | DISPFlagOptimized)
!981 = !DISubroutineType(types: !982)
!982 = !{null, !983}
!983 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !972, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!984 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Derived", file: !2, line: 51, size: 192, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !985, vtableHolder: !972, identifier: "_ZTS7Derived")
!985 = !{!986, !987}
!986 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !984, baseType: !972, flags: DIFlagPublic, extraData: i32 0)
!987 = !DIDerivedType(tag: DW_TAG_member, name: "extraData", scope: !984, file: !2, line: 51, baseType: !30, size: 64, offset: 128, flags: DIFlagPublic)
!988 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "StringHolder", file: !2, line: 63, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !989, identifier: "_ZTS12StringHolder")
!989 = !{!990, !991, !995, !999}
!990 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !988, file: !2, line: 65, baseType: !30, size: 64, flags: DIFlagPublic)
!991 = !DISubprogram(name: "StringHolder", scope: !988, file: !2, line: 66, type: !992, scopeLine: 66, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!992 = !DISubroutineType(types: !993)
!993 = !{null, !994, !30}
!994 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !988, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!995 = !DISubprogram(name: "StringHolder", scope: !988, file: !2, line: 67, type: !996, scopeLine: 67, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!996 = !DISubroutineType(types: !997)
!997 = !{null, !994, !998}
!998 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !988, size: 64)
!999 = !DISubprogram(name: "~StringHolder", scope: !988, file: !2, line: 68, type: !1000, scopeLine: 68, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1000 = !DISubroutineType(types: !1001)
!1001 = !{null, !994}
!1002 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "_ConstructTransaction", scope: !33, file: !32, line: 788, size: 192, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !1003, identifier: "_ZTSNSt3__h6vectorIPcNS_9allocatorIS1_EEE21_ConstructTransactionE")
!1003 = !{!1004, !1005, !1006, !1008, !1012, !1015, !1020}
!1004 = !DIDerivedType(tag: DW_TAG_member, name: "__v_", scope: !1002, file: !32, line: 805, baseType: !249, size: 64)
!1005 = !DIDerivedType(tag: DW_TAG_member, name: "__pos_", scope: !1002, file: !32, line: 806, baseType: !36, size: 64, offset: 64)
!1006 = !DIDerivedType(tag: DW_TAG_member, name: "__new_end_", scope: !1002, file: !32, line: 807, baseType: !1007, size: 64, offset: 128)
!1007 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !397)
!1008 = !DISubprogram(name: "_ConstructTransaction", scope: !1002, file: !32, line: 790, type: !1009, scopeLine: 790, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1009 = !DISubroutineType(types: !1010)
!1010 = !{null, !1011, !249, !221}
!1011 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1002, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1012 = !DISubprogram(name: "~_ConstructTransaction", scope: !1002, file: !32, line: 796, type: !1013, scopeLine: 796, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1013 = !DISubroutineType(types: !1014)
!1014 = !{null, !1011}
!1015 = !DISubprogram(name: "_ConstructTransaction", scope: !1002, file: !32, line: 810, type: !1016, scopeLine: 810, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagOptimized | DISPFlagDeleted)
!1016 = !DISubroutineType(types: !1017)
!1017 = !{null, !1011, !1018}
!1018 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1019, size: 64)
!1019 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1002)
!1020 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE21_ConstructTransactionaSERKS5_", scope: !1002, file: !32, line: 811, type: !1021, scopeLine: 811, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagOptimized | DISPFlagDeleted)
!1021 = !DISubroutineType(types: !1022)
!1022 = !{!1023, !1011, !1018}
!1023 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1002, size: 64)
!1024 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__libcpp_numeric_limits<long, true>", scope: !15, file: !14, line: 198, size: 8, flags: DIFlagTypePassByValue, elements: !1025, templateParams: !1067, identifier: "_ZTSNSt3__h23__libcpp_numeric_limitsIlLb1EEE")
!1025 = !{!1026, !1027, !1028, !1030, !1031, !1032, !1035, !1036, !1037, !1038, !1039, !1040, !1041, !1042, !1043, !1044, !1045, !1046, !1048, !1049, !1050, !1051, !1052, !1053, !1054, !1056, !1059, !1060, !1061, !1062, !1063, !1064, !1065, !1066}
!1026 = !DIDerivedType(tag: DW_TAG_member, name: "is_specialized", scope: !1024, file: !14, line: 203, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 true)
!1027 = !DIDerivedType(tag: DW_TAG_member, name: "is_signed", scope: !1024, file: !14, line: 205, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 true)
!1028 = !DIDerivedType(tag: DW_TAG_member, name: "digits", scope: !1024, file: !14, line: 206, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 63)
!1029 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !17)
!1030 = !DIDerivedType(tag: DW_TAG_member, name: "digits10", scope: !1024, file: !14, line: 207, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 18)
!1031 = !DIDerivedType(tag: DW_TAG_member, name: "max_digits10", scope: !1024, file: !14, line: 208, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1032 = !DIDerivedType(tag: DW_TAG_member, name: "__min", scope: !1024, file: !14, line: 209, baseType: !1033, flags: DIFlagProtected | DIFlagStaticMember, extraData: i64 -9223372036854775808)
!1033 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1034)
!1034 = !DIDerivedType(tag: DW_TAG_typedef, name: "type", scope: !1024, file: !14, line: 201, baseType: !719)
!1035 = !DIDerivedType(tag: DW_TAG_member, name: "__max", scope: !1024, file: !14, line: 210, baseType: !1033, flags: DIFlagProtected | DIFlagStaticMember, extraData: i64 9223372036854775807)
!1036 = !DIDerivedType(tag: DW_TAG_member, name: "is_integer", scope: !1024, file: !14, line: 215, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 true)
!1037 = !DIDerivedType(tag: DW_TAG_member, name: "is_exact", scope: !1024, file: !14, line: 216, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 true)
!1038 = !DIDerivedType(tag: DW_TAG_member, name: "radix", scope: !1024, file: !14, line: 217, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 2)
!1039 = !DIDerivedType(tag: DW_TAG_member, name: "min_exponent", scope: !1024, file: !14, line: 221, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1040 = !DIDerivedType(tag: DW_TAG_member, name: "min_exponent10", scope: !1024, file: !14, line: 222, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1041 = !DIDerivedType(tag: DW_TAG_member, name: "max_exponent", scope: !1024, file: !14, line: 223, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1042 = !DIDerivedType(tag: DW_TAG_member, name: "max_exponent10", scope: !1024, file: !14, line: 224, baseType: !1029, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1043 = !DIDerivedType(tag: DW_TAG_member, name: "has_infinity", scope: !1024, file: !14, line: 226, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1044 = !DIDerivedType(tag: DW_TAG_member, name: "has_quiet_NaN", scope: !1024, file: !14, line: 227, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1045 = !DIDerivedType(tag: DW_TAG_member, name: "has_signaling_NaN", scope: !1024, file: !14, line: 228, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1046 = !DIDerivedType(tag: DW_TAG_member, name: "has_denorm", scope: !1024, file: !14, line: 229, baseType: !1047, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1047 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !13)
!1048 = !DIDerivedType(tag: DW_TAG_member, name: "has_denorm_loss", scope: !1024, file: !14, line: 230, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1049 = !DIDerivedType(tag: DW_TAG_member, name: "is_iec559", scope: !1024, file: !14, line: 236, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1050 = !DIDerivedType(tag: DW_TAG_member, name: "is_bounded", scope: !1024, file: !14, line: 237, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 true)
!1051 = !DIDerivedType(tag: DW_TAG_member, name: "is_modulo", scope: !1024, file: !14, line: 238, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1052 = !DIDerivedType(tag: DW_TAG_member, name: "traps", scope: !1024, file: !14, line: 244, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1053 = !DIDerivedType(tag: DW_TAG_member, name: "tinyness_before", scope: !1024, file: !14, line: 246, baseType: !584, flags: DIFlagProtected | DIFlagStaticMember, extraData: i1 false)
!1054 = !DIDerivedType(tag: DW_TAG_member, name: "round_style", scope: !1024, file: !14, line: 247, baseType: !1055, flags: DIFlagProtected | DIFlagStaticMember, extraData: i32 0)
!1055 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !22)
!1056 = !DISubprogram(name: "min", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE3minB6v15004Ev", scope: !1024, file: !14, line: 211, type: !1057, scopeLine: 211, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1057 = !DISubroutineType(types: !1058)
!1058 = !{!1034}
!1059 = !DISubprogram(name: "max", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE3maxB6v15004Ev", scope: !1024, file: !14, line: 212, type: !1057, scopeLine: 212, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1060 = !DISubprogram(name: "lowest", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE6lowestB6v15004Ev", scope: !1024, file: !14, line: 213, type: !1057, scopeLine: 213, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1061 = !DISubprogram(name: "epsilon", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE7epsilonB6v15004Ev", scope: !1024, file: !14, line: 218, type: !1057, scopeLine: 218, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1062 = !DISubprogram(name: "round_error", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE11round_errorB6v15004Ev", scope: !1024, file: !14, line: 219, type: !1057, scopeLine: 219, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1063 = !DISubprogram(name: "infinity", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE8infinityB6v15004Ev", scope: !1024, file: !14, line: 231, type: !1057, scopeLine: 231, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1064 = !DISubprogram(name: "quiet_NaN", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE9quiet_NaNB6v15004Ev", scope: !1024, file: !14, line: 232, type: !1057, scopeLine: 232, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1065 = !DISubprogram(name: "signaling_NaN", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE13signaling_NaNB6v15004Ev", scope: !1024, file: !14, line: 233, type: !1057, scopeLine: 233, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1066 = !DISubprogram(name: "denorm_min", linkageName: "_ZNSt3__h23__libcpp_numeric_limitsIlLb1EE10denorm_minB6v15004Ev", scope: !1024, file: !14, line: 234, type: !1057, scopeLine: 234, flags: DIFlagProtected | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1067 = !{!1068, !1069}
!1068 = !DITemplateTypeParameter(name: "_Tp", type: !719)
!1069 = !DITemplateValueParameter(type: !58, value: i8 1)
!1070 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pair<std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **> >", scope: !15, file: !1071, line: 40, size: 256, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1072, templateParams: !1107, identifier: "_ZTSNSt3__h4pairINS_16reverse_iteratorIPPcEES4_EE")
!1071 = !DIFile(filename: "pair.h", directory: "")
!1072 = !{!1073, !1074, !1075, !1081, !1085, !1100, !1104}
!1073 = !DIDerivedType(tag: DW_TAG_member, name: "first", scope: !1070, file: !1071, line: 48, baseType: !700, size: 128)
!1074 = !DIDerivedType(tag: DW_TAG_member, name: "second", scope: !1070, file: !1071, line: 49, baseType: !700, size: 128, offset: 128)
!1075 = !DISubprogram(name: "pair", scope: !1070, file: !1071, line: 52, type: !1076, scopeLine: 52, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1076 = !DISubroutineType(types: !1077)
!1077 = !{null, !1078, !1079}
!1078 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1070, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1079 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1080, size: 64)
!1080 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1070)
!1081 = !DISubprogram(name: "pair", scope: !1070, file: !1071, line: 53, type: !1082, scopeLine: 53, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1082 = !DISubroutineType(types: !1083)
!1083 = !{null, !1078, !1084}
!1084 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !1070, size: 64)
!1085 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h4pairINS_16reverse_iteratorIPPcEES4_EaSB6v15004ERKS5_", scope: !1070, file: !1071, line: 262, type: !1086, scopeLine: 262, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1086 = !DISubroutineType(types: !1087)
!1087 = !{!1088, !1078, !1089}
!1088 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1070, size: 64)
!1089 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1090, size: 64)
!1090 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1091)
!1091 = !DIDerivedType(tag: DW_TAG_typedef, name: "type", scope: !1093, file: !1092, line: 39, baseType: !1070)
!1092 = !DIFile(filename: "conditional.h", directory: "")
!1093 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "conditional<true, std::__h::pair<std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **> >, std::__h::__nat>", scope: !15, file: !1092, line: 39, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !1094, identifier: "_ZTSNSt3__h11conditionalILb1ENS_4pairINS_16reverse_iteratorIPPcEES5_EENS_5__natEEE")
!1094 = !{!1095, !1096, !1097}
!1095 = !DITemplateValueParameter(name: "_Bp", type: !58, value: i8 1)
!1096 = !DITemplateTypeParameter(name: "_If", type: !1070)
!1097 = !DITemplateTypeParameter(name: "_Then", type: !1098)
!1098 = !DICompositeType(tag: DW_TAG_structure_type, name: "__nat", scope: !15, file: !1099, line: 20, size: 8, flags: DIFlagFwdDecl, identifier: "_ZTSNSt3__h5__natE")
!1099 = !DIFile(filename: "nat.h", directory: "")
!1100 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h4pairINS_16reverse_iteratorIPPcEES4_EaSB6v15004EOS5_", scope: !1070, file: !1071, line: 275, type: !1101, scopeLine: 275, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1101 = !DISubroutineType(types: !1102)
!1102 = !{!1088, !1078, !1103}
!1103 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !1091, size: 64)
!1104 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h4pairINS_16reverse_iteratorIPPcEES4_E4swapB6v15004ERS5_", scope: !1070, file: !1071, line: 300, type: !1105, scopeLine: 300, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1105 = !DISubroutineType(types: !1106)
!1106 = !{null, !1078, !1088}
!1107 = !{!1108, !1109}
!1108 = !DITemplateTypeParameter(name: "_T1", type: !700)
!1109 = !DITemplateTypeParameter(name: "_T2", type: !700)
!1110 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pair<char **, char **>", scope: !15, file: !1071, line: 40, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1111, templateParams: !1141, identifier: "_ZTSNSt3__h4pairIPPcS2_EE")
!1111 = !{!1112, !1113, !1114, !1120, !1124, !1134, !1138}
!1112 = !DIDerivedType(tag: DW_TAG_member, name: "first", scope: !1110, file: !1071, line: 48, baseType: !67, size: 64)
!1113 = !DIDerivedType(tag: DW_TAG_member, name: "second", scope: !1110, file: !1071, line: 49, baseType: !67, size: 64, offset: 64)
!1114 = !DISubprogram(name: "pair", scope: !1110, file: !1071, line: 52, type: !1115, scopeLine: 52, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1115 = !DISubroutineType(types: !1116)
!1116 = !{null, !1117, !1118}
!1117 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1110, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1118 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1119, size: 64)
!1119 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1110)
!1120 = !DISubprogram(name: "pair", scope: !1110, file: !1071, line: 53, type: !1121, scopeLine: 53, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1121 = !DISubroutineType(types: !1122)
!1122 = !{null, !1117, !1123}
!1123 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !1110, size: 64)
!1124 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h4pairIPPcS2_EaSB6v15004ERKS3_", scope: !1110, file: !1071, line: 262, type: !1125, scopeLine: 262, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1125 = !DISubroutineType(types: !1126)
!1126 = !{!1127, !1117, !1128}
!1127 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1110, size: 64)
!1128 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1129, size: 64)
!1129 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1130)
!1130 = !DIDerivedType(tag: DW_TAG_typedef, name: "type", scope: !1131, file: !1092, line: 39, baseType: !1110)
!1131 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "conditional<true, std::__h::pair<char **, char **>, std::__h::__nat>", scope: !15, file: !1092, line: 39, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !1132, identifier: "_ZTSNSt3__h11conditionalILb1ENS_4pairIPPcS3_EENS_5__natEEE")
!1132 = !{!1095, !1133, !1097}
!1133 = !DITemplateTypeParameter(name: "_If", type: !1110)
!1134 = !DISubprogram(name: "operator=", linkageName: "_ZNSt3__h4pairIPPcS2_EaSB6v15004EOS3_", scope: !1110, file: !1071, line: 275, type: !1135, scopeLine: 275, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1135 = !DISubroutineType(types: !1136)
!1136 = !{!1127, !1117, !1137}
!1137 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !1130, size: 64)
!1138 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h4pairIPPcS2_E4swapB6v15004ERS3_", scope: !1110, file: !1071, line: 300, type: !1139, scopeLine: 300, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1139 = !DISubroutineType(types: !1140)
!1140 = !{null, !1117, !1127}
!1141 = !{!206, !1142}
!1142 = !DITemplateTypeParameter(name: "_T2", type: !67)
!1143 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__non_trivial_if<true, std::__h::allocator<char> >", scope: !15, file: !47, line: 76, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1144, templateParams: !1149, identifier: "_ZTSNSt3__h16__non_trivial_ifILb1ENS_9allocatorIcEEEE")
!1144 = !{!1145}
!1145 = !DISubprogram(name: "__non_trivial_if", scope: !1143, file: !47, line: 78, type: !1146, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1146 = !DISubroutineType(types: !1147)
!1147 = !{null, !1148}
!1148 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1143, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1149 = !{!57, !1150}
!1150 = !DITemplateTypeParameter(name: "_Unique", type: !1151)
!1151 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "allocator<char>", scope: !15, file: !47, line: 87, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1152, templateParams: !971, identifier: "_ZTSNSt3__h9allocatorIcEE")
!1152 = !{!1153, !1154, !1158, !1161, !1164, !1171, !1178, !1181, !1184}
!1153 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !1151, baseType: !1143, extraData: i32 0)
!1154 = !DISubprogram(name: "allocator", scope: !1151, file: !47, line: 99, type: !1155, scopeLine: 99, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1155 = !DISubroutineType(types: !1156)
!1156 = !{null, !1157}
!1157 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1151, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1158 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorIcE8allocateB6v15004Em", scope: !1151, file: !47, line: 106, type: !1159, scopeLine: 106, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1159 = !DISubroutineType(types: !1160)
!1160 = !{!30, !1157, !68}
!1161 = !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h9allocatorIcE10deallocateB6v15004EPcm", scope: !1151, file: !47, line: 124, type: !1162, scopeLine: 124, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1162 = !DISubroutineType(types: !1163)
!1163 = !{null, !1157, !30, !68}
!1164 = !DISubprogram(name: "address", linkageName: "_ZNKSt3__h9allocatorIcE7addressB6v15004ERc", scope: !1151, file: !47, line: 145, type: !1165, scopeLine: 145, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1165 = !DISubroutineType(types: !1166)
!1166 = !{!1167, !1168, !1170}
!1167 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !1151, file: !47, line: 134, baseType: !30)
!1168 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1169, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1169 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1151)
!1170 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !1151, file: !47, line: 136, baseType: !881)
!1171 = !DISubprogram(name: "address", linkageName: "_ZNKSt3__h9allocatorIcE7addressB6v15004ERKc", scope: !1151, file: !47, line: 149, type: !1172, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1172 = !DISubroutineType(types: !1173)
!1173 = !{!1174, !1168, !1176}
!1174 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_pointer", scope: !1151, file: !47, line: 135, baseType: !1175)
!1175 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!1176 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !1151, file: !47, line: 137, baseType: !1177)
!1177 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !4, size: 64)
!1178 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorIcE8allocateB6v15004EmPKv", scope: !1151, file: !47, line: 154, type: !1179, scopeLine: 154, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1179 = !DISubroutineType(types: !1180)
!1180 = !{!30, !1157, !68, !93}
!1181 = !DISubprogram(name: "max_size", linkageName: "_ZNKSt3__h9allocatorIcE8max_sizeB6v15004Ev", scope: !1151, file: !47, line: 158, type: !1182, scopeLine: 158, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1182 = !DISubroutineType(types: !1183)
!1183 = !{!98, !1168}
!1184 = !DISubprogram(name: "destroy", linkageName: "_ZNSt3__h9allocatorIcE7destroyB6v15004EPc", scope: !1151, file: !47, line: 169, type: !1185, scopeLine: 169, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1185 = !DISubroutineType(types: !1186)
!1186 = !{null, !1157, !1167}
!1187 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<std::__h::__compressed_pair<char *, void (*)(void *)>, 0, false>", scope: !15, file: !114, line: 30, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1188, templateParams: !1208, identifier: "_ZTSNSt3__h22__compressed_pair_elemINS_17__compressed_pairIPcPFvPvEEELi0ELb0EEE")
!1188 = !{!1189, !1190, !1194, !1197, !1201}
!1189 = !DIDerivedType(tag: DW_TAG_member, name: "__value_", scope: !1187, file: !114, line: 53, baseType: !825, size: 128, flags: DIFlagPrivate)
!1190 = !DISubprogram(name: "__compressed_pair_elem", scope: !1187, file: !114, line: 35, type: !1191, scopeLine: 35, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1191 = !DISubroutineType(types: !1192)
!1192 = !{null, !1193, !124}
!1193 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1187, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1194 = !DISubprogram(name: "__compressed_pair_elem", scope: !1187, file: !114, line: 36, type: !1195, scopeLine: 36, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1195 = !DISubroutineType(types: !1196)
!1196 = !{null, !1193, !129}
!1197 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemINS_17__compressed_pairIPcPFvPvEEELi0ELb0EE5__getB6v15004Ev", scope: !1187, file: !114, line: 49, type: !1198, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1198 = !DISubroutineType(types: !1199)
!1199 = !{!1200, !1193}
!1200 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !1187, file: !114, line: 32, baseType: !856)
!1201 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemINS_17__compressed_pairIPcPFvPvEEELi0ELb0EE5__getB6v15004Ev", scope: !1187, file: !114, line: 50, type: !1202, scopeLine: 50, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1202 = !DISubroutineType(types: !1203)
!1203 = !{!1204, !1206}
!1204 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !1187, file: !114, line: 33, baseType: !1205)
!1205 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !837, size: 64)
!1206 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1207, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1207 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1187)
!1208 = !{!1209, !145, !146}
!1209 = !DITemplateTypeParameter(name: "_Tp", type: !825)
!1210 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__compressed_pair_elem<std::__h::allocator<char>, 1, true>", scope: !15, file: !114, line: 57, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1211, templateParams: !1235, identifier: "_ZTSNSt3__h22__compressed_pair_elemINS_9allocatorIcEELi1ELb1EEE")
!1211 = !{!1212, !1213, !1217, !1220, !1223, !1228}
!1212 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !1210, baseType: !1151, flags: DIFlagPrivate, extraData: i32 0)
!1213 = !DISubprogram(name: "__compressed_pair_elem", scope: !1210, file: !114, line: 63, type: !1214, scopeLine: 63, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1214 = !DISubroutineType(types: !1215)
!1215 = !{null, !1216}
!1216 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1210, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1217 = !DISubprogram(name: "__compressed_pair_elem", scope: !1210, file: !114, line: 64, type: !1218, scopeLine: 64, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1218 = !DISubroutineType(types: !1219)
!1219 = !{null, !1216, !124}
!1220 = !DISubprogram(name: "__compressed_pair_elem", scope: !1210, file: !114, line: 65, type: !1221, scopeLine: 65, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1221 = !DISubroutineType(types: !1222)
!1222 = !{null, !1216, !129}
!1223 = !DISubprogram(name: "__get", linkageName: "_ZNSt3__h22__compressed_pair_elemINS_9allocatorIcEELi1ELb1EE5__getB6v15004Ev", scope: !1210, file: !114, line: 78, type: !1224, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1224 = !DISubroutineType(types: !1225)
!1225 = !{!1226, !1216}
!1226 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !1210, file: !114, line: 59, baseType: !1227)
!1227 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1151, size: 64)
!1228 = !DISubprogram(name: "__get", linkageName: "_ZNKSt3__h22__compressed_pair_elemINS_9allocatorIcEELi1ELb1EE5__getB6v15004Ev", scope: !1210, file: !114, line: 79, type: !1229, scopeLine: 79, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1229 = !DISubroutineType(types: !1230)
!1230 = !{!1231, !1233}
!1231 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !1210, file: !114, line: 60, baseType: !1232)
!1232 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1169, size: 64)
!1233 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1234, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1234 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1210)
!1235 = !{!1236, !175, !176}
!1236 = !DITemplateTypeParameter(name: "_Tp", type: !1151)
!1237 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__shared_ptr_pointer<char *, void (*)(void *), std::__h::allocator<char> >", scope: !15, file: !914, line: 230, size: 320, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !1238, vtableHolder: !1293, templateParams: !1294, identifier: "_ZTSNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEE")
!1238 = !{!1239, !1240, !1276, !1280, !1289, !1292}
!1239 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !1237, baseType: !921, flags: DIFlagPublic, extraData: i32 0)
!1240 = !DIDerivedType(tag: DW_TAG_member, name: "__data_", scope: !1237, file: !914, line: 233, baseType: !1241, size: 128, offset: 192)
!1241 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "__compressed_pair<std::__h::__compressed_pair<char *, void (*)(void *)>, std::__h::allocator<char> >", scope: !15, file: !114, line: 83, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1242, templateParams: !1273, identifier: "_ZTSNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEEE")
!1242 = !{!1243, !1244, !1245, !1249, !1254, !1257, !1260, !1265, !1269}
!1243 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !1241, baseType: !1187, extraData: i32 0)
!1244 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !1241, baseType: !1210, extraData: i32 0)
!1245 = !DISubprogram(name: "first", linkageName: "_ZNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE5firstB6v15004Ev", scope: !1241, file: !114, line: 120, type: !1246, scopeLine: 120, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1246 = !DISubroutineType(types: !1247)
!1247 = !{!1200, !1248}
!1248 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1241, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1249 = !DISubprogram(name: "first", linkageName: "_ZNKSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE5firstB6v15004Ev", scope: !1241, file: !114, line: 125, type: !1250, scopeLine: 125, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1250 = !DISubroutineType(types: !1251)
!1251 = !{!1204, !1252}
!1252 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1253, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1253 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1241)
!1254 = !DISubprogram(name: "second", linkageName: "_ZNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE6secondB6v15004Ev", scope: !1241, file: !114, line: 130, type: !1255, scopeLine: 130, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1255 = !DISubroutineType(types: !1256)
!1256 = !{!1226, !1248}
!1257 = !DISubprogram(name: "second", linkageName: "_ZNKSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE6secondB6v15004Ev", scope: !1241, file: !114, line: 135, type: !1258, scopeLine: 135, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1258 = !DISubroutineType(types: !1259)
!1259 = !{!1231, !1252}
!1260 = !DISubprogram(name: "__get_first_base", linkageName: "_ZNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE16__get_first_baseB6v15004EPS8_", scope: !1241, file: !114, line: 140, type: !1261, scopeLine: 140, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1261 = !DISubroutineType(types: !1262)
!1262 = !{!1263, !1264}
!1263 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1187, size: 64)
!1264 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1241, size: 64)
!1265 = !DISubprogram(name: "__get_second_base", linkageName: "_ZNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE17__get_second_baseB6v15004EPS8_", scope: !1241, file: !114, line: 144, type: !1266, scopeLine: 144, flags: DIFlagPublic | DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!1266 = !DISubroutineType(types: !1267)
!1267 = !{!1268, !1264}
!1268 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1210, size: 64)
!1269 = !DISubprogram(name: "swap", linkageName: "_ZNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEE4swapB6v15004ERS8_", scope: !1241, file: !114, line: 149, type: !1270, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1270 = !DISubroutineType(types: !1271)
!1271 = !{null, !1248, !1272}
!1272 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1241, size: 64)
!1273 = !{!1274, !1275}
!1274 = !DITemplateTypeParameter(name: "_T1", type: !825)
!1275 = !DITemplateTypeParameter(name: "_T2", type: !1151)
!1276 = !DISubprogram(name: "__shared_ptr_pointer", scope: !1237, file: !914, line: 236, type: !1277, scopeLine: 236, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1277 = !DISubroutineType(types: !1278)
!1278 = !{null, !1279, !30, !796, !1151}
!1279 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1237, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1280 = !DISubprogram(name: "__get_deleter", linkageName: "_ZNKSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE13__get_deleterERKSt9type_info", scope: !1237, file: !914, line: 240, type: !1281, scopeLine: 240, containingType: !1237, virtualIndex: 3, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual | DISPFlagOptimized)
!1281 = !DISubroutineType(types: !1282)
!1282 = !{!93, !1283, !1285}
!1283 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1284, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1284 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1237)
!1285 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1286, size: 64)
!1286 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1287)
!1287 = !DICompositeType(tag: DW_TAG_class_type, name: "type_info", scope: !16, file: !1288, line: 298, size: 128, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSSt9type_info")
!1288 = !DIFile(filename: "typeinfo", directory: "")
!1289 = !DISubprogram(name: "__on_zero_shared", linkageName: "_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE16__on_zero_sharedEv", scope: !1237, file: !914, line: 244, type: !1290, scopeLine: 244, containingType: !1237, virtualIndex: 2, flags: DIFlagPrototyped, spFlags: DISPFlagVirtual | DISPFlagOptimized)
!1290 = !DISubroutineType(types: !1291)
!1291 = !{null, !1279}
!1292 = !DISubprogram(name: "__on_zero_shared_weak", linkageName: "_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE21__on_zero_shared_weakEv", scope: !1237, file: !914, line: 245, type: !1290, scopeLine: 245, containingType: !1237, virtualIndex: 4, flags: DIFlagPrototyped, spFlags: DISPFlagVirtual | DISPFlagOptimized)
!1293 = !DICompositeType(tag: DW_TAG_class_type, name: "__shared_count", scope: !15, file: !914, line: 147, size: 128, flags: DIFlagFwdDecl | DIFlagNonTrivial, identifier: "_ZTSNSt3__h14__shared_countE")
!1294 = !{!103, !912, !1295}
!1295 = !DITemplateTypeParameter(name: "_Alloc", type: !1151)
!1296 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__non_trivial_if<true, std::__h::allocator<std::__h::__shared_ptr_pointer<char *, void (*)(void *), std::__h::allocator<char> > > >", scope: !15, file: !47, line: 76, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1297, templateParams: !1302, identifier: "_ZTSNSt3__h16__non_trivial_ifILb1ENS_9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS1_IcEEEEEEEE")
!1297 = !{!1298}
!1298 = !DISubprogram(name: "__non_trivial_if", scope: !1296, file: !47, line: 78, type: !1299, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1299 = !DISubroutineType(types: !1300)
!1300 = !{null, !1301}
!1301 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1296, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1302 = !{!57, !1303}
!1303 = !DITemplateTypeParameter(name: "_Unique", type: !1304)
!1304 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "allocator<std::__h::__shared_ptr_pointer<char *, void (*)(void *), std::__h::allocator<char> > >", scope: !15, file: !47, line: 87, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !1305, templateParams: !1342, identifier: "_ZTSNSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEEE")
!1305 = !{!1306, !1307, !1311, !1315, !1318, !1326, !1333, !1336, !1339}
!1306 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !1304, baseType: !1296, extraData: i32 0)
!1307 = !DISubprogram(name: "allocator", scope: !1304, file: !47, line: 99, type: !1308, scopeLine: 99, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1308 = !DISubroutineType(types: !1309)
!1309 = !{null, !1310}
!1310 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1304, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1311 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE8allocateB6v15004Em", scope: !1304, file: !47, line: 106, type: !1312, scopeLine: 106, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1312 = !DISubroutineType(types: !1313)
!1313 = !{!1314, !1310, !68}
!1314 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1237, size: 64)
!1315 = !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE10deallocateB6v15004EPS7_m", scope: !1304, file: !47, line: 124, type: !1316, scopeLine: 124, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1316 = !DISubroutineType(types: !1317)
!1317 = !{null, !1310, !1314, !68}
!1318 = !DISubprogram(name: "address", linkageName: "_ZNKSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE7addressB6v15004ERS7_", scope: !1304, file: !47, line: 145, type: !1319, scopeLine: 145, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1319 = !DISubroutineType(types: !1320)
!1320 = !{!1321, !1322, !1324}
!1321 = !DIDerivedType(tag: DW_TAG_typedef, name: "pointer", scope: !1304, file: !47, line: 134, baseType: !1314)
!1322 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1323, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!1323 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1304)
!1324 = !DIDerivedType(tag: DW_TAG_typedef, name: "reference", scope: !1304, file: !47, line: 136, baseType: !1325)
!1325 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1237, size: 64)
!1326 = !DISubprogram(name: "address", linkageName: "_ZNKSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE7addressB6v15004ERKS7_", scope: !1304, file: !47, line: 149, type: !1327, scopeLine: 149, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1327 = !DISubroutineType(types: !1328)
!1328 = !{!1329, !1322, !1331}
!1329 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_pointer", scope: !1304, file: !47, line: 135, baseType: !1330)
!1330 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1284, size: 64)
!1331 = !DIDerivedType(tag: DW_TAG_typedef, name: "const_reference", scope: !1304, file: !47, line: 137, baseType: !1332)
!1332 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !1284, size: 64)
!1333 = !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE8allocateB6v15004EmPKv", scope: !1304, file: !47, line: 154, type: !1334, scopeLine: 154, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1334 = !DISubroutineType(types: !1335)
!1335 = !{!1314, !1310, !68, !93}
!1336 = !DISubprogram(name: "max_size", linkageName: "_ZNKSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE8max_sizeB6v15004Ev", scope: !1304, file: !47, line: 158, type: !1337, scopeLine: 158, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1337 = !DISubroutineType(types: !1338)
!1338 = !{!98, !1322}
!1339 = !DISubprogram(name: "destroy", linkageName: "_ZNSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE7destroyB6v15004EPS7_", scope: !1304, file: !47, line: 169, type: !1340, scopeLine: 169, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1340 = !DISubroutineType(types: !1341)
!1341 = !{null, !1310, !1321}
!1342 = !{!1343}
!1343 = !DITemplateTypeParameter(name: "_Tp", type: !1237)
!1344 = !{!0, !8, !1345, !1347}
!1345 = !DIGlobalVariableExpression(var: !1346, expr: !DIExpression())
!1346 = distinct !DIGlobalVariable(name: "g_sink_idx", linkageName: "_ZL10g_sink_idx", scope: !10, file: !2, line: 15, type: !17, isLocal: true, isDefinition: true)
!1347 = !DIGlobalVariableExpression(var: !1348, expr: !DIExpression())
!1348 = distinct !DIGlobalVariable(scope: null, file: !32, line: 862, type: !1349, isLocal: true, isDefinition: true)
!1349 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 56, elements: !1350)
!1350 = !{!1351}
!1351 = !DISubrange(count: 7)
!1352 = !{!1353, !1357, !1361, !1367, !1374, !1379, !1383, !1387, !1391, !1397, !1402, !1407, !1411, !1415, !1419, !1424, !1426, !1431, !1435, !1437, !1441, !1445, !1449, !1454, !1458, !1460, !1464, !1466, !1473, !1477, !1482, !1486, !1490, !1494, !1498, !1500, !1504, !1511, !1515, !1519, !1527, !1529, !1531, !1533, !1540, !1544, !1548, !1552, !1554, !1556, !1560, !1564, !1568, !1570, !1574, !1579, !1583, !1587, !1591, !1593, !1595, !1597, !1599, !1601, !1605, !1609, !1611, !1612, !1613, !1617, !1621, !1624, !1626, !1628, !1631, !1634, !1636, !1638, !1641, !1643, !1645, !1647, !1649, !1651, !1653, !1655, !1657, !1660, !1662, !1664, !1666, !1668, !1670, !1672, !1674, !1676, !1678, !1680, !1684, !1690, !1692, !1694, !1696, !1697, !1702, !1704, !1706, !1710, !1712, !1714, !1716, !1718, !1720, !1722, !1724, !1729, !1733, !1735, !1737, !1742, !1747, !1749, !1751, !1753, !1755, !1757, !1759, !1761, !1763, !1765, !1767, !1769, !1771, !1773, !1775, !1777, !1779, !1783, !1785, !1787, !1789, !1793, !1795, !1799, !1801, !1803, !1805, !1807, !1811, !1813, !1815, !1819, !1821, !1823, !1827, !1829, !1833, !1835, !1837, !1841, !1843, !1845, !1847, !1849, !1851, !1853, !1857, !1859, !1861, !1863, !1865, !1867, !1869, !1871, !1875, !1879, !1881, !1883, !1885, !1887, !1889, !1891, !1893, !1895, !1897, !1899, !1901, !1903, !1905, !1907, !1909, !1911, !1913, !1915, !1917, !1921, !1923, !1925, !1927, !1931, !1933, !1937, !1939, !1941, !1943, !1945, !1949, !1951, !1955, !1957, !1959, !1961, !1963, !1967, !1969, !1971, !1975, !1977, !1979, !1981, !1986, !1989, !1990, !1992, !1995, !1999, !2003, !2008, !2013, !2019, !2025, !2029, !2031}
!1353 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1354, file: !1356, line: 94)
!1354 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !1355, line: 58, baseType: !70)
!1355 = !DIFile(filename: "alltypes.h", directory: "", checksumkind: CSK_MD5, checksum: "1071e718a958c5a168e8e771d1f30b89")
!1356 = !DIFile(filename: "cstdlib", directory: "")
!1357 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1358, file: !1356, line: 95)
!1358 = !DIDerivedType(tag: DW_TAG_typedef, name: "div_t", file: !1359, line: 65, baseType: !1360)
!1359 = !DIFile(filename: "stdlib.h", directory: "", checksumkind: CSK_MD5, checksum: "4ae56b2feb06fe30283b2148e55e1d18")
!1360 = !DICompositeType(tag: DW_TAG_structure_type, file: !1359, line: 65, size: 64, flags: DIFlagFwdDecl, identifier: "_ZTS5div_t")
!1361 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1362, file: !1356, line: 96)
!1362 = !DIDerivedType(tag: DW_TAG_typedef, name: "ldiv_t", file: !1359, line: 66, baseType: !1363)
!1363 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1359, line: 66, size: 128, flags: DIFlagTypePassByValue, elements: !1364, identifier: "_ZTS6ldiv_t")
!1364 = !{!1365, !1366}
!1365 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !1363, file: !1359, line: 66, baseType: !719, size: 64)
!1366 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !1363, file: !1359, line: 66, baseType: !719, size: 64, offset: 64)
!1367 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1368, file: !1356, line: 97)
!1368 = !DIDerivedType(tag: DW_TAG_typedef, name: "lldiv_t", file: !1359, line: 67, baseType: !1369)
!1369 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1359, line: 67, size: 128, flags: DIFlagTypePassByValue, elements: !1370, identifier: "_ZTS7lldiv_t")
!1370 = !{!1371, !1373}
!1371 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !1369, file: !1359, line: 67, baseType: !1372, size: 64)
!1372 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!1373 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !1369, file: !1359, line: 67, baseType: !1372, size: 64, offset: 64)
!1374 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1375, file: !1356, line: 98)
!1375 = !DISubprogram(name: "atof", scope: !1359, file: !1359, line: 26, type: !1376, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1376 = !DISubroutineType(types: !1377)
!1377 = !{!1378, !1175}
!1378 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!1379 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1380, file: !1356, line: 99)
!1380 = !DISubprogram(name: "atoi", scope: !1359, file: !1359, line: 23, type: !1381, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1381 = !DISubroutineType(types: !1382)
!1382 = !{!17, !1175}
!1383 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1384, file: !1356, line: 100)
!1384 = !DISubprogram(name: "atol", scope: !1359, file: !1359, line: 24, type: !1385, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1385 = !DISubroutineType(types: !1386)
!1386 = !{!719, !1175}
!1387 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1388, file: !1356, line: 101)
!1388 = !DISubprogram(name: "atoll", scope: !1359, file: !1359, line: 25, type: !1389, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1389 = !DISubroutineType(types: !1390)
!1390 = !{!1372, !1175}
!1391 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1392, file: !1356, line: 102)
!1392 = !DISubprogram(name: "strtod", scope: !1359, file: !1359, line: 29, type: !1393, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1393 = !DISubroutineType(types: !1394)
!1394 = !{!1378, !1395, !1396}
!1395 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !1175)
!1396 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !67)
!1397 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1398, file: !1356, line: 103)
!1398 = !DISubprogram(name: "strtof", scope: !1359, file: !1359, line: 28, type: !1399, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1399 = !DISubroutineType(types: !1400)
!1400 = !{!1401, !1395, !1396}
!1401 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!1402 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1403, file: !1356, line: 104)
!1403 = !DISubprogram(name: "strtold", scope: !1359, file: !1359, line: 30, type: !1404, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1404 = !DISubroutineType(types: !1405)
!1405 = !{!1406, !1395, !1396}
!1406 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!1407 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1408, file: !1356, line: 105)
!1408 = !DISubprogram(name: "strtol", scope: !1359, file: !1359, line: 32, type: !1409, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1409 = !DISubroutineType(types: !1410)
!1410 = !{!719, !1395, !1396, !17}
!1411 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1412, file: !1356, line: 106)
!1412 = !DISubprogram(name: "strtoll", scope: !1359, file: !1359, line: 34, type: !1413, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1413 = !DISubroutineType(types: !1414)
!1414 = !{!1372, !1395, !1396, !17}
!1415 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1416, file: !1356, line: 107)
!1416 = !DISubprogram(name: "strtoul", scope: !1359, file: !1359, line: 33, type: !1417, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1417 = !DISubroutineType(types: !1418)
!1418 = !{!70, !1395, !1396, !17}
!1419 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1420, file: !1356, line: 108)
!1420 = !DISubprogram(name: "strtoull", scope: !1359, file: !1359, line: 35, type: !1421, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1421 = !DISubroutineType(types: !1422)
!1422 = !{!1423, !1395, !1396, !17}
!1423 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!1424 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1425, file: !1356, line: 109)
!1425 = !DISubprogram(name: "rand", scope: !1359, file: !1359, line: 37, type: !977, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1426 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1427, file: !1356, line: 110)
!1427 = !DISubprogram(name: "srand", scope: !1359, file: !1359, line: 38, type: !1428, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1428 = !DISubroutineType(types: !1429)
!1429 = !{null, !1430}
!1430 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!1431 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1432, file: !1356, line: 111)
!1432 = !DISubprogram(name: "calloc", scope: !1359, file: !1359, line: 41, type: !1433, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1433 = !DISubroutineType(types: !1434)
!1434 = !{!691, !1354, !1354}
!1435 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1436, file: !1356, line: 112)
!1436 = !DISubprogram(name: "free", scope: !1359, file: !1359, line: 43, type: !797, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1437 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1438, file: !1356, line: 113)
!1438 = !DISubprogram(name: "malloc", scope: !1359, file: !1359, line: 40, type: !1439, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1439 = !DISubroutineType(types: !1440)
!1440 = !{!691, !1354}
!1441 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1442, file: !1356, line: 114)
!1442 = !DISubprogram(name: "realloc", scope: !1359, file: !1359, line: 42, type: !1443, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1443 = !DISubroutineType(types: !1444)
!1444 = !{!691, !691, !1354}
!1445 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1446, file: !1356, line: 115)
!1446 = !DISubprogram(name: "abort", scope: !1359, file: !1359, line: 46, type: !1447, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!1447 = !DISubroutineType(types: !1448)
!1448 = !{null}
!1449 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1450, file: !1356, line: 116)
!1450 = !DISubprogram(name: "atexit", scope: !1359, file: !1359, line: 48, type: !1451, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1451 = !DISubroutineType(types: !1452)
!1452 = !{!17, !1453}
!1453 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1447, size: 64)
!1454 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1455, file: !1356, line: 117)
!1455 = !DISubprogram(name: "exit", scope: !1359, file: !1359, line: 49, type: !1456, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!1456 = !DISubroutineType(types: !1457)
!1457 = !{null, !17}
!1458 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1459, file: !1356, line: 118)
!1459 = !DISubprogram(name: "_Exit", scope: !1359, file: !1359, line: 50, type: !1456, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!1460 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1461, file: !1356, line: 119)
!1461 = !DISubprogram(name: "getenv", scope: !1359, file: !1359, line: 54, type: !1462, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1462 = !DISubroutineType(types: !1463)
!1463 = !{!30, !1175}
!1464 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1465, file: !1356, line: 120)
!1465 = !DISubprogram(name: "system", scope: !1359, file: !1359, line: 56, type: !1381, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1466 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1467, file: !1356, line: 121)
!1467 = !DISubprogram(name: "bsearch", scope: !1359, file: !1359, line: 58, type: !1468, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1468 = !DISubroutineType(types: !1469)
!1469 = !{!691, !93, !93, !1354, !1354, !1470}
!1470 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1471, size: 64)
!1471 = !DISubroutineType(types: !1472)
!1472 = !{!17, !93, !93}
!1473 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1474, file: !1356, line: 122)
!1474 = !DISubprogram(name: "qsort", scope: !1359, file: !1359, line: 59, type: !1475, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1475 = !DISubroutineType(types: !1476)
!1476 = !{null, !691, !1354, !1354, !1470}
!1477 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1478, file: !1356, line: 123)
!1478 = !DISubprogram(name: "abs", linkageName: "_Z3absB6v15004e", scope: !1479, file: !1479, line: 129, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1479 = !DIFile(filename: "stdlib.h", directory: "")
!1480 = !DISubroutineType(types: !1481)
!1481 = !{!1406, !1406}
!1482 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1483, file: !1356, line: 124)
!1483 = !DISubprogram(name: "labs", scope: !1359, file: !1359, line: 62, type: !1484, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1484 = !DISubroutineType(types: !1485)
!1485 = !{!719, !719}
!1486 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1487, file: !1356, line: 125)
!1487 = !DISubprogram(name: "llabs", scope: !1359, file: !1359, line: 63, type: !1488, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1488 = !DISubroutineType(types: !1489)
!1489 = !{!1372, !1372}
!1490 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1491, file: !1356, line: 126)
!1491 = !DISubprogram(name: "div", linkageName: "_Z3divB6v15004xx", scope: !1479, file: !1479, line: 152, type: !1492, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1492 = !DISubroutineType(types: !1493)
!1493 = !{!1368, !1372, !1372}
!1494 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1495, file: !1356, line: 127)
!1495 = !DISubprogram(name: "ldiv", scope: !1359, file: !1359, line: 70, type: !1496, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1496 = !DISubroutineType(types: !1497)
!1497 = !{!1362, !719, !719}
!1498 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1499, file: !1356, line: 128)
!1499 = !DISubprogram(name: "lldiv", scope: !1359, file: !1359, line: 71, type: !1492, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1500 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1501, file: !1356, line: 129)
!1501 = !DISubprogram(name: "mblen", scope: !1359, file: !1359, line: 73, type: !1502, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1502 = !DISubroutineType(types: !1503)
!1503 = !{!17, !1175, !1354}
!1504 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1505, file: !1356, line: 130)
!1505 = !DISubprogram(name: "mbtowc", scope: !1359, file: !1359, line: 74, type: !1506, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1506 = !DISubroutineType(types: !1507)
!1507 = !{!17, !1508, !1395, !1354}
!1508 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !1509)
!1509 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1510, size: 64)
!1510 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!1511 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1512, file: !1356, line: 131)
!1512 = !DISubprogram(name: "wctomb", scope: !1359, file: !1359, line: 75, type: !1513, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1513 = !DISubroutineType(types: !1514)
!1514 = !{!17, !30, !1510}
!1515 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1516, file: !1356, line: 132)
!1516 = !DISubprogram(name: "mbstowcs", scope: !1359, file: !1359, line: 76, type: !1517, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1517 = !DISubroutineType(types: !1518)
!1518 = !{!1354, !1508, !1395, !1354}
!1519 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1520, file: !1356, line: 133)
!1520 = !DISubprogram(name: "wcstombs", scope: !1359, file: !1359, line: 77, type: !1521, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1521 = !DISubroutineType(types: !1522)
!1522 = !{!1354, !1523, !1524, !1354}
!1523 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !30)
!1524 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !1525)
!1525 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1526, size: 64)
!1526 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1510)
!1527 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1528, file: !1356, line: 135)
!1528 = !DISubprogram(name: "at_quick_exit", scope: !1359, file: !1359, line: 51, type: !1451, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1529 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1530, file: !1356, line: 136)
!1530 = !DISubprogram(name: "quick_exit", scope: !1359, file: !1359, line: 52, type: !1456, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!1531 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1354, file: !1532, line: 69)
!1532 = !DIFile(filename: "cstring", directory: "")
!1533 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1534, file: !1532, line: 70)
!1534 = !DISubprogram(name: "memcpy", scope: !1535, file: !1535, line: 27, type: !1536, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1535 = !DIFile(filename: "string.h", directory: "", checksumkind: CSK_MD5, checksum: "eb1bf98d1059ccc3c197a450734602f7")
!1536 = !DISubroutineType(types: !1537)
!1537 = !{!691, !1538, !1539, !1354}
!1538 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !691)
!1539 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !93)
!1540 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1541, file: !1532, line: 71)
!1541 = !DISubprogram(name: "memmove", scope: !1535, file: !1535, line: 28, type: !1542, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1542 = !DISubroutineType(types: !1543)
!1543 = !{!691, !691, !93, !1354}
!1544 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1545, file: !1532, line: 72)
!1545 = !DISubprogram(name: "strcpy", scope: !1535, file: !1535, line: 33, type: !1546, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1546 = !DISubroutineType(types: !1547)
!1547 = !{!30, !1523, !1395}
!1548 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1549, file: !1532, line: 73)
!1549 = !DISubprogram(name: "strncpy", scope: !1535, file: !1535, line: 34, type: !1550, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1550 = !DISubroutineType(types: !1551)
!1551 = !{!30, !1523, !1395, !1354}
!1552 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1553, file: !1532, line: 74)
!1553 = !DISubprogram(name: "strcat", scope: !1535, file: !1535, line: 36, type: !1546, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1554 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1555, file: !1532, line: 75)
!1555 = !DISubprogram(name: "strncat", scope: !1535, file: !1535, line: 37, type: !1550, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1556 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1557, file: !1532, line: 76)
!1557 = !DISubprogram(name: "memcmp", scope: !1535, file: !1535, line: 30, type: !1558, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1558 = !DISubroutineType(types: !1559)
!1559 = !{!17, !93, !93, !1354}
!1560 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1561, file: !1532, line: 77)
!1561 = !DISubprogram(name: "strcmp", scope: !1535, file: !1535, line: 39, type: !1562, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1562 = !DISubroutineType(types: !1563)
!1563 = !{!17, !1175, !1175}
!1564 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1565, file: !1532, line: 78)
!1565 = !DISubprogram(name: "strncmp", scope: !1535, file: !1535, line: 40, type: !1566, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1566 = !DISubroutineType(types: !1567)
!1567 = !{!17, !1175, !1175, !1354}
!1568 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1569, file: !1532, line: 79)
!1569 = !DISubprogram(name: "strcoll", scope: !1535, file: !1535, line: 42, type: !1562, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1570 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1571, file: !1532, line: 80)
!1571 = !DISubprogram(name: "strxfrm", scope: !1535, file: !1535, line: 43, type: !1572, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1572 = !DISubroutineType(types: !1573)
!1573 = !{!1354, !1523, !1395, !1354}
!1574 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1575, file: !1532, line: 81)
!1575 = !DISubprogram(name: "memchr", linkageName: "_Z6memchrB6v15004Ua9enable_ifILb1EEPvim", scope: !1576, file: !1576, line: 98, type: !1577, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1576 = !DIFile(filename: "string.h", directory: "")
!1577 = !DISubroutineType(types: !1578)
!1578 = !{!691, !691, !17, !1354}
!1579 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1580, file: !1532, line: 82)
!1580 = !DISubprogram(name: "strchr", linkageName: "_Z6strchrB6v15004Ua9enable_ifILb1EEPci", scope: !1576, file: !1576, line: 77, type: !1581, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1581 = !DISubroutineType(types: !1582)
!1582 = !{!30, !30, !17}
!1583 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1584, file: !1532, line: 83)
!1584 = !DISubprogram(name: "strcspn", scope: !1535, file: !1535, line: 48, type: !1585, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1585 = !DISubroutineType(types: !1586)
!1586 = !{!1354, !1175, !1175}
!1587 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1588, file: !1532, line: 84)
!1588 = !DISubprogram(name: "strpbrk", linkageName: "_Z7strpbrkB6v15004Ua9enable_ifILb1EEPcPKc", scope: !1576, file: !1576, line: 84, type: !1589, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1589 = !DISubroutineType(types: !1590)
!1590 = !{!30, !30, !1175}
!1591 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1592, file: !1532, line: 85)
!1592 = !DISubprogram(name: "strrchr", linkageName: "_Z7strrchrB6v15004Ua9enable_ifILb1EEPci", scope: !1576, file: !1576, line: 91, type: !1581, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1593 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1594, file: !1532, line: 86)
!1594 = !DISubprogram(name: "strspn", scope: !1535, file: !1535, line: 49, type: !1585, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1595 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1596, file: !1532, line: 87)
!1596 = !DISubprogram(name: "strstr", linkageName: "_Z6strstrB6v15004Ua9enable_ifILb1EEPcPKc", scope: !1576, file: !1576, line: 105, type: !1589, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1597 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1598, file: !1532, line: 88)
!1598 = !DISubprogram(name: "strtok", scope: !1535, file: !1535, line: 52, type: !1546, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1599 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1600, file: !1532, line: 89)
!1600 = !DISubprogram(name: "memset", scope: !1535, file: !1535, line: 29, type: !1577, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1601 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1602, file: !1532, line: 90)
!1602 = !DISubprogram(name: "strerror", scope: !1535, file: !1535, line: 56, type: !1603, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1603 = !DISubroutineType(types: !1604)
!1604 = !{!30, !17}
!1605 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1606, file: !1532, line: 91)
!1606 = !DISubprogram(name: "strlen", scope: !1535, file: !1535, line: 54, type: !1607, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1607 = !DISubroutineType(types: !1608)
!1608 = !{!1354, !1175}
!1609 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !875, file: !1610, line: 50)
!1610 = !DIFile(filename: "cstddef", directory: "")
!1611 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !763, file: !1610, line: 51)
!1612 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !68, file: !1610, line: 52)
!1613 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1614, file: !1610, line: 55)
!1614 = !DIDerivedType(tag: DW_TAG_typedef, name: "max_align_t", file: !1615, line: 24, baseType: !1616)
!1615 = !DIFile(filename: "__stddef_max_align_t.h", directory: "", checksumkind: CSK_MD5, checksum: "48e8e2456f77e6cda35d245130fa7259")
!1616 = !DICompositeType(tag: DW_TAG_structure_type, file: !1615, line: 19, size: 256, flags: DIFlagFwdDecl, identifier: "_ZTS11max_align_t")
!1617 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1618, file: !1620, line: 153)
!1618 = !DIDerivedType(tag: DW_TAG_typedef, name: "int8_t", file: !1355, line: 104, baseType: !1619)
!1619 = !DIBasicType(name: "signed char", size: 8, encoding: DW_ATE_signed_char)
!1620 = !DIFile(filename: "cstdint", directory: "")
!1621 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1622, file: !1620, line: 154)
!1622 = !DIDerivedType(tag: DW_TAG_typedef, name: "int16_t", file: !1355, line: 109, baseType: !1623)
!1623 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!1624 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1625, file: !1620, line: 155)
!1625 = !DIDerivedType(tag: DW_TAG_typedef, name: "int32_t", file: !1355, line: 114, baseType: !17)
!1626 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1627, file: !1620, line: 156)
!1627 = !DIDerivedType(tag: DW_TAG_typedef, name: "int64_t", file: !1355, line: 119, baseType: !719)
!1628 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1629, file: !1620, line: 158)
!1629 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint8_t", file: !1355, line: 129, baseType: !1630)
!1630 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!1631 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1632, file: !1620, line: 159)
!1632 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint16_t", file: !1355, line: 134, baseType: !1633)
!1633 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!1634 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1635, file: !1620, line: 160)
!1635 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !1355, line: 139, baseType: !1430)
!1636 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1637, file: !1620, line: 161)
!1637 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint64_t", file: !1355, line: 144, baseType: !70)
!1638 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1639, file: !1620, line: 163)
!1639 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least8_t", file: !1640, line: 25, baseType: !1618)
!1640 = !DIFile(filename: "stdint.h", directory: "", checksumkind: CSK_MD5, checksum: "19b17d487ee68139328911f286d556b7")
!1641 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1642, file: !1620, line: 164)
!1642 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least16_t", file: !1640, line: 26, baseType: !1622)
!1643 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1644, file: !1620, line: 165)
!1644 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least32_t", file: !1640, line: 27, baseType: !1625)
!1645 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1646, file: !1620, line: 166)
!1646 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least64_t", file: !1640, line: 28, baseType: !1627)
!1647 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1648, file: !1620, line: 168)
!1648 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least8_t", file: !1640, line: 33, baseType: !1629)
!1649 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1650, file: !1620, line: 169)
!1650 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least16_t", file: !1640, line: 34, baseType: !1632)
!1651 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1652, file: !1620, line: 170)
!1652 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least32_t", file: !1640, line: 35, baseType: !1635)
!1653 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1654, file: !1620, line: 171)
!1654 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least64_t", file: !1640, line: 36, baseType: !1637)
!1655 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1656, file: !1620, line: 173)
!1656 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast8_t", file: !1640, line: 22, baseType: !1618)
!1657 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1658, file: !1620, line: 174)
!1658 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast16_t", file: !1659, line: 1, baseType: !1625)
!1659 = !DIFile(filename: "stdint.h", directory: "", checksumkind: CSK_MD5, checksum: "8bf94eb4172b2ccfe00f3e4301006d98")
!1660 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1661, file: !1620, line: 175)
!1661 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast32_t", file: !1659, line: 2, baseType: !1625)
!1662 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1663, file: !1620, line: 176)
!1663 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast64_t", file: !1640, line: 23, baseType: !1627)
!1664 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1665, file: !1620, line: 178)
!1665 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast8_t", file: !1640, line: 30, baseType: !1629)
!1666 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1667, file: !1620, line: 179)
!1667 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast16_t", file: !1659, line: 3, baseType: !1635)
!1668 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1669, file: !1620, line: 180)
!1669 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast32_t", file: !1659, line: 4, baseType: !1635)
!1670 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1671, file: !1620, line: 181)
!1671 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast64_t", file: !1640, line: 31, baseType: !1637)
!1672 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1673, file: !1620, line: 183)
!1673 = !DIDerivedType(tag: DW_TAG_typedef, name: "intptr_t", file: !1355, line: 78, baseType: !719)
!1674 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1675, file: !1620, line: 184)
!1675 = !DIDerivedType(tag: DW_TAG_typedef, name: "uintptr_t", file: !1355, line: 63, baseType: !70)
!1676 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1677, file: !1620, line: 186)
!1677 = !DIDerivedType(tag: DW_TAG_typedef, name: "intmax_t", file: !1355, line: 124, baseType: !719)
!1678 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1679, file: !1620, line: 187)
!1679 = !DIDerivedType(tag: DW_TAG_typedef, name: "uintmax_t", file: !1355, line: 154, baseType: !70)
!1680 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1681, file: !1683, line: 40)
!1681 = !DIDerivedType(tag: DW_TAG_typedef, name: "mbstate_t", file: !1355, line: 345, baseType: !1682)
!1682 = !DICompositeType(tag: DW_TAG_structure_type, name: "__mbstate_t", file: !1355, line: 345, size: 64, flags: DIFlagFwdDecl, identifier: "_ZTS11__mbstate_t")
!1683 = !DIFile(filename: "__mbstate_t.h", directory: "")
!1684 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1685, file: !1689, line: 325)
!1685 = !DISubprogram(name: "isinf", linkageName: "_Z5isinfB6v15004e", scope: !1686, file: !1686, line: 515, type: !1687, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1686 = !DIFile(filename: "math.h", directory: "")
!1687 = !DISubroutineType(types: !1688)
!1688 = !{!58, !1406}
!1689 = !DIFile(filename: "cmath", directory: "")
!1690 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1691, file: !1689, line: 326)
!1691 = !DISubprogram(name: "isnan", linkageName: "_Z5isnanB6v15004e", scope: !1686, file: !1686, line: 563, type: !1687, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1692 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1693, file: !1689, line: 336)
!1693 = !DIDerivedType(tag: DW_TAG_typedef, name: "float_t", file: !1355, line: 38, baseType: !1401)
!1694 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1695, file: !1689, line: 337)
!1695 = !DIDerivedType(tag: DW_TAG_typedef, name: "double_t", file: !1355, line: 43, baseType: !1378)
!1696 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1478, file: !1689, line: 339)
!1697 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1698, file: !1689, line: 342)
!1698 = !DISubprogram(name: "acosf", scope: !1699, file: !1699, line: 136, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1699 = !DIFile(filename: "math.h", directory: "", checksumkind: CSK_MD5, checksum: "95f1091aa5c39bead584db7c48e5913d")
!1700 = !DISubroutineType(types: !1701)
!1701 = !{!1401, !1401}
!1702 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1703, file: !1689, line: 344)
!1703 = !DISubprogram(name: "asinf", scope: !1699, file: !1699, line: 144, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1704 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1705, file: !1689, line: 346)
!1705 = !DISubprogram(name: "atanf", scope: !1699, file: !1699, line: 152, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1706 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1707, file: !1689, line: 348)
!1707 = !DISubprogram(name: "atan2f", scope: !1699, file: !1699, line: 156, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1708 = !DISubroutineType(types: !1709)
!1709 = !{!1401, !1401, !1401}
!1710 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1711, file: !1689, line: 350)
!1711 = !DISubprogram(name: "ceilf", scope: !1699, file: !1699, line: 168, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1712 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1713, file: !1689, line: 352)
!1713 = !DISubprogram(name: "cosf", scope: !1699, file: !1699, line: 176, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1714 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1715, file: !1689, line: 354)
!1715 = !DISubprogram(name: "coshf", scope: !1699, file: !1699, line: 180, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1716 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1717, file: !1689, line: 357)
!1717 = !DISubprogram(name: "expf", scope: !1699, file: !1699, line: 192, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1718 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1719, file: !1689, line: 360)
!1719 = !DISubprogram(name: "fabsf", scope: !1699, file: !1699, line: 204, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1720 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1721, file: !1689, line: 362)
!1721 = !DISubprogram(name: "floorf", scope: !1699, file: !1699, line: 212, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1722 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1723, file: !1689, line: 365)
!1723 = !DISubprogram(name: "fmodf", scope: !1699, file: !1699, line: 228, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1724 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1725, file: !1689, line: 368)
!1725 = !DISubprogram(name: "frexpf", scope: !1699, file: !1699, line: 232, type: !1726, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1726 = !DISubroutineType(types: !1727)
!1727 = !{!1401, !1401, !1728}
!1728 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64)
!1729 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1730, file: !1689, line: 370)
!1730 = !DISubprogram(name: "ldexpf", scope: !1699, file: !1699, line: 244, type: !1731, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1731 = !DISubroutineType(types: !1732)
!1732 = !{!1401, !1401, !17}
!1733 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1734, file: !1689, line: 373)
!1734 = !DISubprogram(name: "logf", scope: !1699, file: !1699, line: 260, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1735 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1736, file: !1689, line: 376)
!1736 = !DISubprogram(name: "log10f", scope: !1699, file: !1699, line: 264, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1737 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1738, file: !1689, line: 377)
!1738 = !DISubprogram(name: "modf", linkageName: "_Z4modfB6v15004ePe", scope: !1686, file: !1686, line: 996, type: !1739, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1739 = !DISubroutineType(types: !1740)
!1740 = !{!1406, !1406, !1741}
!1741 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1406, size: 64)
!1742 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1743, file: !1689, line: 378)
!1743 = !DISubprogram(name: "modff", scope: !1699, file: !1699, line: 288, type: !1744, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1744 = !DISubroutineType(types: !1745)
!1745 = !{!1401, !1401, !1746}
!1746 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1401, size: 64)
!1747 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1748, file: !1689, line: 381)
!1748 = !DISubprogram(name: "powf", scope: !1699, file: !1699, line: 308, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1749 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1750, file: !1689, line: 384)
!1750 = !DISubprogram(name: "sinf", scope: !1699, file: !1699, line: 336, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1751 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1752, file: !1689, line: 386)
!1752 = !DISubprogram(name: "sinhf", scope: !1699, file: !1699, line: 340, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1753 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1754, file: !1689, line: 389)
!1754 = !DISubprogram(name: "sqrtf", scope: !1699, file: !1699, line: 344, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1755 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1756, file: !1689, line: 391)
!1756 = !DISubprogram(name: "tanf", scope: !1699, file: !1699, line: 348, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1757 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1758, file: !1689, line: 394)
!1758 = !DISubprogram(name: "tanhf", scope: !1699, file: !1699, line: 352, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1759 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1760, file: !1689, line: 397)
!1760 = !DISubprogram(name: "acoshf", scope: !1699, file: !1699, line: 140, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1761 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1762, file: !1689, line: 399)
!1762 = !DISubprogram(name: "asinhf", scope: !1699, file: !1699, line: 148, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1763 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1764, file: !1689, line: 401)
!1764 = !DISubprogram(name: "atanhf", scope: !1699, file: !1699, line: 160, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1765 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1766, file: !1689, line: 403)
!1766 = !DISubprogram(name: "cbrtf", scope: !1699, file: !1699, line: 164, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1767 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1768, file: !1689, line: 406)
!1768 = !DISubprogram(name: "copysignf", scope: !1699, file: !1699, line: 172, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1769 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1770, file: !1689, line: 409)
!1770 = !DISubprogram(name: "erff", scope: !1699, file: !1699, line: 184, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1771 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1772, file: !1689, line: 411)
!1772 = !DISubprogram(name: "erfcf", scope: !1699, file: !1699, line: 188, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1773 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1774, file: !1689, line: 413)
!1774 = !DISubprogram(name: "exp2f", scope: !1699, file: !1699, line: 196, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1775 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1776, file: !1689, line: 415)
!1776 = !DISubprogram(name: "expm1f", scope: !1699, file: !1699, line: 200, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1777 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1778, file: !1689, line: 417)
!1778 = !DISubprogram(name: "fdimf", scope: !1699, file: !1699, line: 208, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1779 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1780, file: !1689, line: 418)
!1780 = !DISubprogram(name: "fmaf", scope: !1699, file: !1699, line: 216, type: !1781, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1781 = !DISubroutineType(types: !1782)
!1782 = !{!1401, !1401, !1401, !1401}
!1783 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1784, file: !1689, line: 421)
!1784 = !DISubprogram(name: "fmaxf", scope: !1699, file: !1699, line: 220, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1785 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1786, file: !1689, line: 423)
!1786 = !DISubprogram(name: "fminf", scope: !1699, file: !1699, line: 224, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1787 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1788, file: !1689, line: 425)
!1788 = !DISubprogram(name: "hypotf", scope: !1699, file: !1699, line: 236, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1789 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1790, file: !1689, line: 427)
!1790 = !DISubprogram(name: "ilogbf", scope: !1699, file: !1699, line: 240, type: !1791, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1791 = !DISubroutineType(types: !1792)
!1792 = !{!17, !1401}
!1793 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1794, file: !1689, line: 429)
!1794 = !DISubprogram(name: "lgammaf", scope: !1699, file: !1699, line: 248, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1795 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1796, file: !1689, line: 431)
!1796 = !DISubprogram(name: "llrintf", scope: !1699, file: !1699, line: 252, type: !1797, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1797 = !DISubroutineType(types: !1798)
!1798 = !{!1372, !1401}
!1799 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1800, file: !1689, line: 433)
!1800 = !DISubprogram(name: "llroundf", scope: !1699, file: !1699, line: 256, type: !1797, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1801 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1802, file: !1689, line: 435)
!1802 = !DISubprogram(name: "log1pf", scope: !1699, file: !1699, line: 268, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1803 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1804, file: !1689, line: 437)
!1804 = !DISubprogram(name: "log2f", scope: !1699, file: !1699, line: 272, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1805 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1806, file: !1689, line: 439)
!1806 = !DISubprogram(name: "logbf", scope: !1699, file: !1699, line: 276, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1807 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1808, file: !1689, line: 441)
!1808 = !DISubprogram(name: "lrintf", scope: !1699, file: !1699, line: 280, type: !1809, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1809 = !DISubroutineType(types: !1810)
!1810 = !{!719, !1401}
!1811 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1812, file: !1689, line: 443)
!1812 = !DISubprogram(name: "lroundf", scope: !1699, file: !1699, line: 284, type: !1809, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1813 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1814, file: !1689, line: 445)
!1814 = !DISubprogram(name: "nan", scope: !1699, file: !1699, line: 291, type: !1376, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1815 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1816, file: !1689, line: 446)
!1816 = !DISubprogram(name: "nanf", scope: !1699, file: !1699, line: 292, type: !1817, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1817 = !DISubroutineType(types: !1818)
!1818 = !{!1401, !1175}
!1819 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1820, file: !1689, line: 449)
!1820 = !DISubprogram(name: "nearbyintf", scope: !1699, file: !1699, line: 296, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1821 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1822, file: !1689, line: 451)
!1822 = !DISubprogram(name: "nextafterf", scope: !1699, file: !1699, line: 300, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1823 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1824, file: !1689, line: 453)
!1824 = !DISubprogram(name: "nexttowardf", scope: !1699, file: !1699, line: 304, type: !1825, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1825 = !DISubroutineType(types: !1826)
!1826 = !{!1401, !1401, !1406}
!1827 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1828, file: !1689, line: 455)
!1828 = !DISubprogram(name: "remainderf", scope: !1699, file: !1699, line: 312, type: !1708, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1829 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1830, file: !1689, line: 457)
!1830 = !DISubprogram(name: "remquof", scope: !1699, file: !1699, line: 316, type: !1831, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1831 = !DISubroutineType(types: !1832)
!1832 = !{!1401, !1401, !1401, !1728}
!1833 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1834, file: !1689, line: 459)
!1834 = !DISubprogram(name: "rintf", scope: !1699, file: !1699, line: 320, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1835 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1836, file: !1689, line: 461)
!1836 = !DISubprogram(name: "roundf", scope: !1699, file: !1699, line: 324, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1837 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1838, file: !1689, line: 463)
!1838 = !DISubprogram(name: "scalblnf", scope: !1699, file: !1699, line: 328, type: !1839, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1839 = !DISubroutineType(types: !1840)
!1840 = !{!1401, !1401, !719}
!1841 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1842, file: !1689, line: 465)
!1842 = !DISubprogram(name: "scalbnf", scope: !1699, file: !1699, line: 332, type: !1731, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1843 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1844, file: !1689, line: 467)
!1844 = !DISubprogram(name: "tgammaf", scope: !1699, file: !1699, line: 356, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1845 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1846, file: !1689, line: 469)
!1846 = !DISubprogram(name: "truncf", scope: !1699, file: !1699, line: 360, type: !1700, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1847 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1848, file: !1689, line: 471)
!1848 = !DISubprogram(name: "acosl", scope: !1699, file: !1699, line: 137, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1849 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1850, file: !1689, line: 472)
!1850 = !DISubprogram(name: "asinl", scope: !1699, file: !1699, line: 145, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1851 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1852, file: !1689, line: 473)
!1852 = !DISubprogram(name: "atanl", scope: !1699, file: !1699, line: 153, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1853 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1854, file: !1689, line: 474)
!1854 = !DISubprogram(name: "atan2l", scope: !1699, file: !1699, line: 157, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1855 = !DISubroutineType(types: !1856)
!1856 = !{!1406, !1406, !1406}
!1857 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1858, file: !1689, line: 475)
!1858 = !DISubprogram(name: "ceill", scope: !1699, file: !1699, line: 169, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1859 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1860, file: !1689, line: 476)
!1860 = !DISubprogram(name: "cosl", scope: !1699, file: !1699, line: 177, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1861 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1862, file: !1689, line: 477)
!1862 = !DISubprogram(name: "coshl", scope: !1699, file: !1699, line: 181, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1863 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1864, file: !1689, line: 478)
!1864 = !DISubprogram(name: "expl", scope: !1699, file: !1699, line: 193, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1865 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1866, file: !1689, line: 479)
!1866 = !DISubprogram(name: "fabsl", scope: !1699, file: !1699, line: 205, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1867 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1868, file: !1689, line: 480)
!1868 = !DISubprogram(name: "floorl", scope: !1699, file: !1699, line: 213, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1869 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1870, file: !1689, line: 481)
!1870 = !DISubprogram(name: "fmodl", scope: !1699, file: !1699, line: 229, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1871 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1872, file: !1689, line: 482)
!1872 = !DISubprogram(name: "frexpl", scope: !1699, file: !1699, line: 233, type: !1873, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1873 = !DISubroutineType(types: !1874)
!1874 = !{!1406, !1406, !1728}
!1875 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1876, file: !1689, line: 483)
!1876 = !DISubprogram(name: "ldexpl", scope: !1699, file: !1699, line: 245, type: !1877, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1877 = !DISubroutineType(types: !1878)
!1878 = !{!1406, !1406, !17}
!1879 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1880, file: !1689, line: 484)
!1880 = !DISubprogram(name: "logl", scope: !1699, file: !1699, line: 261, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1881 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1882, file: !1689, line: 485)
!1882 = !DISubprogram(name: "log10l", scope: !1699, file: !1699, line: 265, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1883 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1884, file: !1689, line: 486)
!1884 = !DISubprogram(name: "modfl", scope: !1699, file: !1699, line: 289, type: !1739, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1885 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1886, file: !1689, line: 487)
!1886 = !DISubprogram(name: "powl", scope: !1699, file: !1699, line: 309, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1887 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1888, file: !1689, line: 488)
!1888 = !DISubprogram(name: "sinl", scope: !1699, file: !1699, line: 337, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1889 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1890, file: !1689, line: 489)
!1890 = !DISubprogram(name: "sinhl", scope: !1699, file: !1699, line: 341, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1891 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1892, file: !1689, line: 490)
!1892 = !DISubprogram(name: "sqrtl", scope: !1699, file: !1699, line: 345, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1893 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1894, file: !1689, line: 491)
!1894 = !DISubprogram(name: "tanl", scope: !1699, file: !1699, line: 349, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1895 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1896, file: !1689, line: 493)
!1896 = !DISubprogram(name: "tanhl", scope: !1699, file: !1699, line: 353, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1897 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1898, file: !1689, line: 494)
!1898 = !DISubprogram(name: "acoshl", scope: !1699, file: !1699, line: 141, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1899 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1900, file: !1689, line: 495)
!1900 = !DISubprogram(name: "asinhl", scope: !1699, file: !1699, line: 149, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1901 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1902, file: !1689, line: 496)
!1902 = !DISubprogram(name: "atanhl", scope: !1699, file: !1699, line: 161, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1903 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1904, file: !1689, line: 497)
!1904 = !DISubprogram(name: "cbrtl", scope: !1699, file: !1699, line: 165, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1905 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1906, file: !1689, line: 499)
!1906 = !DISubprogram(name: "copysignl", scope: !1699, file: !1699, line: 173, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1907 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1908, file: !1689, line: 501)
!1908 = !DISubprogram(name: "erfl", scope: !1699, file: !1699, line: 185, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1909 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1910, file: !1689, line: 502)
!1910 = !DISubprogram(name: "erfcl", scope: !1699, file: !1699, line: 189, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1911 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1912, file: !1689, line: 503)
!1912 = !DISubprogram(name: "exp2l", scope: !1699, file: !1699, line: 197, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1913 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1914, file: !1689, line: 504)
!1914 = !DISubprogram(name: "expm1l", scope: !1699, file: !1699, line: 201, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1915 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1916, file: !1689, line: 505)
!1916 = !DISubprogram(name: "fdiml", scope: !1699, file: !1699, line: 209, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1917 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1918, file: !1689, line: 506)
!1918 = !DISubprogram(name: "fmal", scope: !1699, file: !1699, line: 217, type: !1919, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1919 = !DISubroutineType(types: !1920)
!1920 = !{!1406, !1406, !1406, !1406}
!1921 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1922, file: !1689, line: 507)
!1922 = !DISubprogram(name: "fmaxl", scope: !1699, file: !1699, line: 221, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1923 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1924, file: !1689, line: 508)
!1924 = !DISubprogram(name: "fminl", scope: !1699, file: !1699, line: 225, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1925 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1926, file: !1689, line: 509)
!1926 = !DISubprogram(name: "hypotl", scope: !1699, file: !1699, line: 237, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1927 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1928, file: !1689, line: 510)
!1928 = !DISubprogram(name: "ilogbl", scope: !1699, file: !1699, line: 241, type: !1929, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1929 = !DISubroutineType(types: !1930)
!1930 = !{!17, !1406}
!1931 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1932, file: !1689, line: 511)
!1932 = !DISubprogram(name: "lgammal", scope: !1699, file: !1699, line: 249, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1933 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1934, file: !1689, line: 512)
!1934 = !DISubprogram(name: "llrintl", scope: !1699, file: !1699, line: 253, type: !1935, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1935 = !DISubroutineType(types: !1936)
!1936 = !{!1372, !1406}
!1937 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1938, file: !1689, line: 513)
!1938 = !DISubprogram(name: "llroundl", scope: !1699, file: !1699, line: 257, type: !1935, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1939 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1940, file: !1689, line: 514)
!1940 = !DISubprogram(name: "log1pl", scope: !1699, file: !1699, line: 269, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1941 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1942, file: !1689, line: 515)
!1942 = !DISubprogram(name: "log2l", scope: !1699, file: !1699, line: 273, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1943 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1944, file: !1689, line: 516)
!1944 = !DISubprogram(name: "logbl", scope: !1699, file: !1699, line: 277, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1945 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1946, file: !1689, line: 517)
!1946 = !DISubprogram(name: "lrintl", scope: !1699, file: !1699, line: 281, type: !1947, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1947 = !DISubroutineType(types: !1948)
!1948 = !{!719, !1406}
!1949 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1950, file: !1689, line: 518)
!1950 = !DISubprogram(name: "lroundl", scope: !1699, file: !1699, line: 285, type: !1947, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1951 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1952, file: !1689, line: 519)
!1952 = !DISubprogram(name: "nanl", scope: !1699, file: !1699, line: 293, type: !1953, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1953 = !DISubroutineType(types: !1954)
!1954 = !{!1406, !1175}
!1955 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1956, file: !1689, line: 520)
!1956 = !DISubprogram(name: "nearbyintl", scope: !1699, file: !1699, line: 297, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1957 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1958, file: !1689, line: 521)
!1958 = !DISubprogram(name: "nextafterl", scope: !1699, file: !1699, line: 301, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1959 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1960, file: !1689, line: 522)
!1960 = !DISubprogram(name: "nexttowardl", scope: !1699, file: !1699, line: 305, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1961 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1962, file: !1689, line: 523)
!1962 = !DISubprogram(name: "remainderl", scope: !1699, file: !1699, line: 313, type: !1855, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1963 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1964, file: !1689, line: 524)
!1964 = !DISubprogram(name: "remquol", scope: !1699, file: !1699, line: 317, type: !1965, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1965 = !DISubroutineType(types: !1966)
!1966 = !{!1406, !1406, !1406, !1728}
!1967 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1968, file: !1689, line: 525)
!1968 = !DISubprogram(name: "rintl", scope: !1699, file: !1699, line: 321, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1969 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1970, file: !1689, line: 526)
!1970 = !DISubprogram(name: "roundl", scope: !1699, file: !1699, line: 325, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1971 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1972, file: !1689, line: 527)
!1972 = !DISubprogram(name: "scalblnl", scope: !1699, file: !1699, line: 329, type: !1973, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1973 = !DISubroutineType(types: !1974)
!1974 = !{!1406, !1406, !719}
!1975 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1976, file: !1689, line: 528)
!1976 = !DISubprogram(name: "scalbnl", scope: !1699, file: !1699, line: 333, type: !1877, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1977 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1978, file: !1689, line: 529)
!1978 = !DISubprogram(name: "tgammal", scope: !1699, file: !1699, line: 357, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1979 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1980, file: !1689, line: 530)
!1980 = !DISubprogram(name: "truncl", scope: !1699, file: !1699, line: 361, type: !1480, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1981 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !1982, entity: !1983, file: !1985, line: 606)
!1982 = !DINamespace(name: "chrono", scope: !15)
!1983 = !DINamespace(name: "chrono_literals", scope: !1984, exportSymbols: true)
!1984 = !DINamespace(name: "literals", scope: !15, exportSymbols: true)
!1985 = !DIFile(filename: "duration.h", directory: "")
!1986 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1987, file: !1988, line: 58)
!1987 = !DIDerivedType(tag: DW_TAG_typedef, name: "clock_t", file: !1355, line: 227, baseType: !719)
!1988 = !DIFile(filename: "ctime", directory: "")
!1989 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !68, file: !1988, line: 59)
!1990 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1991, file: !1988, line: 60)
!1991 = !DIDerivedType(tag: DW_TAG_typedef, name: "time_t", file: !1355, line: 93, baseType: !719)
!1992 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1993, file: !1988, line: 61)
!1993 = !DICompositeType(tag: DW_TAG_structure_type, name: "tm", file: !1994, line: 40, size: 448, flags: DIFlagFwdDecl, identifier: "_ZTS2tm")
!1994 = !DIFile(filename: "time.h", directory: "", checksumkind: CSK_MD5, checksum: "95e0c9313cab71d7f0433f0217fadb42")
!1995 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !1996, file: !1988, line: 65)
!1996 = !DISubprogram(name: "clock", scope: !1994, file: !1994, line: 54, type: !1997, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!1997 = !DISubroutineType(types: !1998)
!1998 = !{!1987}
!1999 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2000, file: !1988, line: 66)
!2000 = !DISubprogram(name: "difftime", scope: !1994, file: !1994, line: 56, type: !2001, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2001 = !DISubroutineType(types: !2002)
!2002 = !{!1378, !1991, !1991}
!2003 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2004, file: !1988, line: 67)
!2004 = !DISubprogram(name: "mktime", scope: !1994, file: !1994, line: 57, type: !2005, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2005 = !DISubroutineType(types: !2006)
!2006 = !{!1991, !2007}
!2007 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1993, size: 64)
!2008 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2009, file: !1988, line: 68)
!2009 = !DISubprogram(name: "time", scope: !1994, file: !1994, line: 55, type: !2010, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2010 = !DISubroutineType(types: !2011)
!2011 = !{!1991, !2012}
!2012 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1991, size: 64)
!2013 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2014, file: !1988, line: 69)
!2014 = !DISubprogram(name: "asctime", scope: !1994, file: !1994, line: 61, type: !2015, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2015 = !DISubroutineType(types: !2016)
!2016 = !{!30, !2017}
!2017 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2018, size: 64)
!2018 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1993)
!2019 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2020, file: !1988, line: 70)
!2020 = !DISubprogram(name: "ctime", scope: !1994, file: !1994, line: 62, type: !2021, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2021 = !DISubroutineType(types: !2022)
!2022 = !{!30, !2023}
!2023 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2024, size: 64)
!2024 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1991)
!2025 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2026, file: !1988, line: 71)
!2026 = !DISubprogram(name: "gmtime", scope: !1994, file: !1994, line: 59, type: !2027, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2027 = !DISubroutineType(types: !2028)
!2028 = !{!2007, !2023}
!2029 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2030, file: !1988, line: 72)
!2030 = !DISubprogram(name: "localtime", scope: !1994, file: !1994, line: 60, type: !2027, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2031 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !2032, file: !1988, line: 73)
!2032 = !DISubprogram(name: "strftime", scope: !1994, file: !1994, line: 58, type: !2033, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2033 = !DISubroutineType(types: !2034)
!2034 = !{!68, !1523, !68, !1395, !2035}
!2035 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !2017)
!2036 = !DICompositeType(tag: DW_TAG_array_type, baseType: !691, size: 2048, elements: !2037)
!2037 = !{!2038}
!2038 = !DISubrange(count: 32)
!2039 = !{i32 7, !"Dwarf Version", i32 5}
!2040 = !{i32 7, !"ReferenceTracking", i32 1}
!2041 = !{i32 2, !"Debug Info Version", i32 3}
!2042 = !{i32 1, !"wchar_size", i32 4}
!2043 = !{i32 7, !"PIC Level", i32 2}
!2044 = !{i32 7, !"frame-pointer", i32 1}
!2045 = !{!"clang version 15.0.4"}
!2046 = distinct !DISubprogram(name: "test_memcpy_store", linkageName: "_Z17test_memcpy_storev", scope: !2, file: !2, line: 19, type: !1447, scopeLine: 19, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2047)
!2047 = !{!2048, !2049}
!2048 = !DILocalVariable(name: "src", scope: !2046, file: !2, line: 20, type: !691)
!2049 = !DILocalVariable(name: "dst", scope: !2046, file: !2, line: 21, type: !691)
!2050 = !DILocation(line: 20, column: 17, scope: !2046)
!2051 = !{!"src", !"void*"}
!2052 = !DILocation(line: 0, scope: !2046)
!2053 = !DILocation(line: 21, column: 17, scope: !2046)
!2054 = !{!"dst", !"void*"}
!2055 = !DILocation(line: 22, column: 5, scope: !2046)
!2056 = !DILocalVariable(name: "p", arg: 1, scope: !2057, file: !2, line: 16, type: !691)
!2057 = distinct !DISubprogram(name: "save_ptr", linkageName: "_ZL8save_ptrPv", scope: !2, file: !2, line: 16, type: !797, scopeLine: 16, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2058)
!2058 = !{!2056}
!2059 = !DILocation(line: 0, scope: !2057, inlinedAt: !2060)
!2060 = distinct !DILocation(line: 23, column: 5, scope: !2046)
!2061 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2060)
!2062 = !{!2063, !2063, i64 0}
!2063 = !{!"int", !2064, i64 0}
!2064 = !{!"omnipotent char", !2065, i64 0}
!2065 = !{!"Simple C++ TBAA"}
!2066 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2060)
!2067 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2060)
!2068 = !{!2069, !2069, i64 0}
!2069 = !{!"any pointer", !2064, i64 0}
!2070 = !{!"g_sink[]", !"void*"}
!2071 = !DILocation(line: 0, scope: !2057, inlinedAt: !2072)
!2072 = distinct !DILocation(line: 23, column: 20, scope: !2046)
!2073 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2072)
!2074 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2072)
!2075 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2072)
!2076 = !DILocation(line: 24, column: 1, scope: !2046)
!2077 = distinct !DISubprogram(name: "test_strcpy_store", linkageName: "_Z17test_strcpy_storePKc", scope: !2, file: !2, line: 27, type: !1462, scopeLine: 27, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2078)
!2078 = !{!2079, !2080}
!2079 = !DILocalVariable(name: "src", arg: 1, scope: !2077, file: !2, line: 27, type: !1175)
!2080 = !DILocalVariable(name: "dst", scope: !2077, file: !2, line: 28, type: !30)
!2081 = !DILocation(line: 0, scope: !2077)
!2082 = !DILocation(line: 28, column: 24, scope: !2077)
!2083 = !{!"dst", !"char*"}
!2084 = !DILocation(line: 29, column: 5, scope: !2077)
!2085 = !DILocation(line: 0, scope: !2057, inlinedAt: !2086)
!2086 = distinct !DILocation(line: 30, column: 5, scope: !2077)
!2087 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2086)
!2088 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2086)
!2089 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2086)
!2090 = !DILocation(line: 31, column: 5, scope: !2077)
!2091 = distinct !DISubprogram(name: "test_stl_vector_store", linkageName: "_Z21test_stl_vector_storev", scope: !2, file: !2, line: 35, type: !1447, scopeLine: 35, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2092)
!2092 = !{!2093}
!2093 = !DILocalVariable(name: "vec", scope: !2091, file: !2, line: 36, type: !33)
!2094 = !DILocation(line: 36, column: 5, scope: !2091)
!2095 = !DILocation(line: 36, column: 24, scope: !2091)
!2096 = !DILocalVariable(name: "this", arg: 1, scope: !2097, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2097 = distinct !DISubprogram(name: "vector", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEEC2B6v15004Ev", scope: !33, file: !32, line: 365, type: !209, scopeLine: 366, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !208, retainedNodes: !2098)
!2098 = !{!2096}
!2099 = !DILocation(line: 0, scope: !2097, inlinedAt: !2100)
!2100 = distinct !DILocation(line: 36, column: 24, scope: !2091)
!2101 = !DILocation(line: 676, column: 13, scope: !2097, inlinedAt: !2100)
!2102 = !DILocation(line: 37, column: 26, scope: !2091)
!2103 = !{!"0_UNKNOWN_", !"0_UNKNOWN_"}
!2104 = !DILocalVariable(name: "this", arg: 1, scope: !2105, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2105 = distinct !DISubprogram(name: "push_back", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE9push_backB6v15004EOS1_", scope: !33, file: !32, line: 1594, type: !350, scopeLine: 1595, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !349, retainedNodes: !2106)
!2106 = !{!2104, !2107}
!2107 = !DILocalVariable(name: "__x", arg: 2, scope: !2105, file: !32, line: 594, type: !352)
!2108 = !DILocation(line: 0, scope: !2105, inlinedAt: !2109)
!2109 = distinct !DILocation(line: 37, column: 9, scope: !2091)
!2110 = !DILocalVariable(name: "this", arg: 1, scope: !2111, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2111 = distinct !DISubprogram(name: "__end_cap", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE9__end_capB6v15004Ev", scope: !33, file: !32, line: 830, type: !658, scopeLine: 831, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !657, retainedNodes: !2112)
!2112 = !{!2110}
!2113 = !DILocation(line: 0, scope: !2111, inlinedAt: !2114)
!2114 = distinct !DILocation(line: 1596, column: 30, scope: !2115, inlinedAt: !2109)
!2115 = distinct !DILexicalBlock(scope: !2105, file: !32, line: 1596, column: 9)
!2116 = !DILocalVariable(name: "this", arg: 1, scope: !2117, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2117 = distinct !DISubprogram(name: "__push_back_slow_path<char *>", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE21__push_back_slow_pathIS1_EEvOT_", scope: !33, file: !32, line: 1566, type: !2118, scopeLine: 1567, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2122, declaration: !2121, retainedNodes: !2124)
!2118 = !DISubroutineType(types: !2119)
!2119 = !{null, !211, !2120}
!2120 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !30, size: 64)
!2121 = !DISubprogram(name: "__push_back_slow_path<char *>", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE21__push_back_slow_pathIS1_EEvOT_", scope: !33, file: !32, line: 737, type: !2118, scopeLine: 737, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized, templateParams: !2122)
!2122 = !{!2123}
!2123 = !DITemplateTypeParameter(name: "_Up", type: !30)
!2124 = !{!2116, !2125, !2126, !2127}
!2125 = !DILocalVariable(name: "__x", arg: 2, scope: !2117, file: !32, line: 737, type: !2120)
!2126 = !DILocalVariable(name: "__a", scope: !2117, file: !32, line: 1568, type: !653)
!2127 = !DILocalVariable(name: "__v", scope: !2117, file: !32, line: 1569, type: !403)
!2128 = !DILocation(line: 0, scope: !2117, inlinedAt: !2129)
!2129 = distinct !DILocation(line: 1601, column: 9, scope: !2115, inlinedAt: !2109)
!2130 = !DILocalVariable(name: "this", arg: 1, scope: !2131, type: !2146, flags: DIFlagArtificial | DIFlagObjectPointer)
!2131 = distinct !DISubprogram(name: "__split_buffer", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEEC2EmmS4_", scope: !403, file: !404, line: 310, type: !505, scopeLine: 312, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !504, retainedNodes: !2132)
!2132 = !{!2130, !2133, !2134, !2135, !2136}
!2133 = !DILocalVariable(name: "__cap", arg: 2, scope: !2131, file: !404, line: 77, type: !507)
!2134 = !DILocalVariable(name: "__start", arg: 3, scope: !2131, file: !404, line: 77, type: !507)
!2135 = !DILocalVariable(name: "__a", arg: 4, scope: !2131, file: !404, line: 77, type: !478)
!2136 = !DILocalVariable(name: "__allocation", scope: !2137, file: !404, line: 316, type: !2140)
!2137 = distinct !DILexicalBlock(scope: !2138, file: !404, line: 315, column: 12)
!2138 = distinct !DILexicalBlock(scope: !2139, file: !404, line: 313, column: 9)
!2139 = distinct !DILexicalBlock(scope: !2131, file: !404, line: 312, column: 1)
!2140 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__allocation_result<char **>", scope: !15, file: !2141, line: 46, size: 128, flags: DIFlagTypePassByValue, elements: !2142, templateParams: !2145, identifier: "_ZTSNSt3__h19__allocation_resultIPPcEE")
!2141 = !DIFile(filename: "allocate_at_least.h", directory: "")
!2142 = !{!2143, !2144}
!2143 = !DIDerivedType(tag: DW_TAG_member, name: "ptr", scope: !2140, file: !2141, line: 47, baseType: !67, size: 64)
!2144 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !2140, file: !2141, line: 48, baseType: !68, size: 64, offset: 64)
!2145 = !{!720}
!2146 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !403, size: 64)
!2147 = !DILocation(line: 0, scope: !2131, inlinedAt: !2148)
!2148 = distinct !DILocation(line: 1569, column: 49, scope: !2117, inlinedAt: !2129)
!2149 = !DILocalVariable(name: "__alloc", arg: 1, scope: !2150, file: !2141, line: 53, type: !165)
!2150 = distinct !DISubprogram(name: "__allocate_at_least<std::__h::allocator<char *> >", linkageName: "_ZNSt3__h19__allocate_at_leastB6v15004INS_9allocatorIPcEEEENS_19__allocation_resultINS_16allocator_traitsIT_E7pointerEEERS6_m", scope: !15, file: !2141, line: 53, type: !2151, scopeLine: 53, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !109, retainedNodes: !2153)
!2151 = !DISubroutineType(types: !2152)
!2152 = !{!2140, !165, !68}
!2153 = !{!2149, !2154}
!2154 = !DILocalVariable(name: "__n", arg: 2, scope: !2150, file: !2141, line: 53, type: !68)
!2155 = !DILocation(line: 0, scope: !2150, inlinedAt: !2156)
!2156 = distinct !DILocation(line: 316, column: 29, scope: !2137, inlinedAt: !2148)
!2157 = !DILocalVariable(name: "this", arg: 1, scope: !2158, type: !2161, flags: DIFlagArtificial | DIFlagObjectPointer)
!2158 = distinct !DISubprogram(name: "allocate", linkageName: "_ZNSt3__h9allocatorIPcE8allocateB6v15004Em", scope: !46, file: !47, line: 106, type: !65, scopeLine: 106, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !64, retainedNodes: !2159)
!2159 = !{!2157, !2160}
!2160 = !DILocalVariable(name: "__n", arg: 2, scope: !2158, file: !47, line: 106, type: !68)
!2161 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64)
!2162 = !DILocation(line: 0, scope: !2158, inlinedAt: !2163)
!2163 = distinct !DILocation(line: 54, column: 19, scope: !2150, inlinedAt: !2156)
!2164 = !DILocalVariable(name: "__size", arg: 1, scope: !2165, file: !2166, line: 263, type: !68)
!2165 = distinct !DISubprogram(name: "__libcpp_allocate", linkageName: "_ZNSt3__h17__libcpp_allocateB6v15004Emm", scope: !15, file: !2166, line: 263, type: !2167, scopeLine: 263, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2169)
!2166 = !DIFile(filename: "new", directory: "")
!2167 = !DISubroutineType(types: !2168)
!2168 = !{!691, !68, !68}
!2169 = !{!2164, !2170}
!2170 = !DILocalVariable(name: "__align", arg: 2, scope: !2165, file: !2166, line: 263, type: !68)
!2171 = !DILocation(line: 0, scope: !2165, inlinedAt: !2172)
!2172 = distinct !DILocation(line: 112, column: 38, scope: !2173, inlinedAt: !2163)
!2173 = distinct !DILexicalBlock(scope: !2174, file: !47, line: 111, column: 16)
!2174 = distinct !DILexicalBlock(scope: !2158, file: !47, line: 109, column: 13)
!2175 = !DILocalVariable(name: "__args", arg: 1, scope: !2176, file: !2166, line: 244, type: !70)
!2176 = distinct !DISubprogram(name: "__libcpp_operator_new<unsigned long>", linkageName: "_ZNSt3__h21__libcpp_operator_newB6v15004IJmEEEPvDpT_", scope: !15, file: !2166, line: 244, type: !2177, scopeLine: 244, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2180, retainedNodes: !2179)
!2177 = !DISubroutineType(types: !2178)
!2178 = !{!691, !70}
!2179 = !{!2175}
!2180 = !{!2181}
!2181 = !DITemplateValueParameter(tag: DW_TAG_GNU_template_parameter_pack, name: "_Args", value: !2182)
!2182 = !{!2183}
!2183 = !DITemplateTypeParameter(type: !70)
!2184 = !DILocation(line: 0, scope: !2176, inlinedAt: !2185)
!2185 = distinct !DILocation(line: 272, column: 10, scope: !2165, inlinedAt: !2172)
!2186 = !DILocation(line: 246, column: 10, scope: !2176, inlinedAt: !2185)
!2187 = !DILocalVariable(name: "__a", arg: 1, scope: !2188, file: !38, line: 288, type: !44)
!2188 = distinct !DISubprogram(name: "construct<char *, char *, void>", linkageName: "_ZNSt3__h16allocator_traitsINS_9allocatorIPcEEE9constructB6v15004IS2_JS2_EvEEvRS3_PT_DpOT0_", scope: !39, file: !38, line: 288, type: !2189, scopeLine: 288, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2192, declaration: !2191, retainedNodes: !2197)
!2189 = !DISubroutineType(types: !2190)
!2190 = !{null, !44, !67, !2120}
!2191 = !DISubprogram(name: "construct<char *, char *, void>", linkageName: "_ZNSt3__h16allocator_traitsINS_9allocatorIPcEEE9constructB6v15004IS2_JS2_EvEEvRS3_PT_DpOT0_", scope: !39, file: !38, line: 288, type: !2189, scopeLine: 288, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized, templateParams: !2192)
!2192 = !{!103, !2193, !2196}
!2193 = !DITemplateValueParameter(tag: DW_TAG_GNU_template_parameter_pack, name: "_Args", value: !2194)
!2194 = !{!2195}
!2195 = !DITemplateTypeParameter(type: !30)
!2196 = !DITemplateTypeParameter(type: null)
!2197 = !{!2187, !2198, !2199}
!2198 = !DILocalVariable(name: "__p", arg: 2, scope: !2188, file: !38, line: 288, type: !67)
!2199 = !DILocalVariable(name: "__args", arg: 3, scope: !2188, file: !38, line: 288, type: !2120)
!2200 = !DILocation(line: 0, scope: !2188, inlinedAt: !2201)
!2201 = distinct !DILocation(line: 1571, column: 5, scope: !2117, inlinedAt: !2129)
!2202 = !DILocalVariable(name: "this", arg: 1, scope: !2203, type: !2161, flags: DIFlagArtificial | DIFlagObjectPointer)
!2203 = distinct !DISubprogram(name: "construct<char *, char *>", linkageName: "_ZNSt3__h9allocatorIPcE9constructB6v15004IS1_JS1_EEEvPT_DpOT0_", scope: !46, file: !47, line: 164, type: !2204, scopeLine: 164, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2207, declaration: !2206, retainedNodes: !2208)
!2204 = !DISubroutineType(types: !2205)
!2205 = !{null, !63, !67, !2120}
!2206 = !DISubprogram(name: "construct<char *, char *>", linkageName: "_ZNSt3__h9allocatorIPcE9constructB6v15004IS1_JS1_EEEvPT_DpOT0_", scope: !46, file: !47, line: 164, type: !2204, scopeLine: 164, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized, templateParams: !2207)
!2207 = !{!2123, !2193}
!2208 = !{!2202, !2209, !2210}
!2209 = !DILocalVariable(name: "__p", arg: 2, scope: !2203, file: !47, line: 164, type: !67)
!2210 = !DILocalVariable(name: "__args", arg: 3, scope: !2203, file: !47, line: 164, type: !2120)
!2211 = !DILocation(line: 0, scope: !2203, inlinedAt: !2212)
!2212 = distinct !DILocation(line: 290, column: 13, scope: !2188, inlinedAt: !2201)
!2213 = !DILocation(line: 165, column: 9, scope: !2203, inlinedAt: !2212)
!2214 = !{!"__p", !"char*"}
!2215 = !DILocation(line: 0, scope: !695, inlinedAt: !2216)
!2216 = distinct !DILocation(line: 1573, column: 5, scope: !2117, inlinedAt: !2129)
!2217 = !DILocalVariable(name: "__first1", arg: 2, scope: !2218, file: !2219, line: 626, type: !700)
!2218 = distinct !DISubprogram(name: "__uninitialized_allocator_move_if_noexcept<std::__h::allocator<char *>, std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **>, char *, void>", linkageName: "_ZNSt3__h42__uninitialized_allocator_move_if_noexceptB6v15004INS_9allocatorIPcEENS_16reverse_iteratorIPS2_EES6_S2_vEET1_RT_T0_SA_S7_", scope: !15, file: !2219, line: 626, type: !2220, scopeLine: 626, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2226, retainedNodes: !2222)
!2219 = !DIFile(filename: "uninitialized_algorithms.h", directory: "")
!2220 = !DISubroutineType(types: !2221)
!2221 = !{!700, !165, !700, !700, !700}
!2222 = !{!2223, !2217, !2224, !2225}
!2223 = !DILocalVariable(arg: 1, scope: !2218, file: !2219, line: 626, type: !165)
!2224 = !DILocalVariable(name: "__last1", arg: 3, scope: !2218, file: !2219, line: 626, type: !700)
!2225 = !DILocalVariable(name: "__first2", arg: 4, scope: !2218, file: !2219, line: 626, type: !700)
!2226 = !{!110, !2227, !2228, !2229, !2196}
!2227 = !DITemplateTypeParameter(name: "_Iter1", type: !700)
!2228 = !DITemplateTypeParameter(name: "_Iter2", type: !700)
!2229 = !DITemplateTypeParameter(name: "_Type", type: !30)
!2230 = !DILocation(line: 0, scope: !2218, inlinedAt: !2231)
!2231 = distinct !DILocation(line: 924, column: 22, scope: !695, inlinedAt: !2216)
!2232 = !DILocalVariable(name: "__first", arg: 1, scope: !2233, file: !2234, line: 113, type: !700)
!2233 = distinct !DISubprogram(name: "move<std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **> >", linkageName: "_ZNSt3__h4moveB6v15004INS_16reverse_iteratorIPPcEES4_EET0_T_S6_S5_", scope: !15, file: !2234, line: 113, type: !2235, scopeLine: 113, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2240, retainedNodes: !2237)
!2234 = !DIFile(filename: "move.h", directory: "")
!2235 = !DISubroutineType(types: !2236)
!2236 = !{!700, !700, !700, !700}
!2237 = !{!2232, !2238, !2239}
!2238 = !DILocalVariable(name: "__last", arg: 2, scope: !2233, file: !2234, line: 113, type: !700)
!2239 = !DILocalVariable(name: "__result", arg: 3, scope: !2233, file: !2234, line: 113, type: !700)
!2240 = !{!2241, !2242}
!2241 = !DITemplateTypeParameter(name: "_InputIterator", type: !700)
!2242 = !DITemplateTypeParameter(name: "_OutputIterator", type: !700)
!2243 = !DILocation(line: 0, scope: !2233, inlinedAt: !2244)
!2244 = distinct !DILocation(line: 635, column: 12, scope: !2245, inlinedAt: !2231)
!2245 = distinct !DILexicalBlock(scope: !2246, file: !2219, line: 634, column: 10)
!2246 = distinct !DILexicalBlock(scope: !2218, file: !2219, line: 627, column: 7)
!2247 = !DILocalVariable(name: "__first", arg: 1, scope: !2248, file: !2234, line: 96, type: !700)
!2248 = distinct !DISubprogram(name: "__move<std::__h::_ClassicAlgPolicy, std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **> >", linkageName: "_ZNSt3__h6__moveB6v15004INS_17_ClassicAlgPolicyENS_16reverse_iteratorIPPcEES5_S5_EENS_9enable_ifIXaaaasr21is_copy_constructibleIT0_EE5valuesr21is_copy_constructibleIT1_EE5valuesr21is_copy_constructibleIT2_EE5valueENS_4pairIS7_S9_EEE4typeES7_S8_S9_", scope: !15, file: !2234, line: 96, type: !2249, scopeLine: 96, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2260, retainedNodes: !2256)
!2249 = !DISubroutineType(types: !2250)
!2250 = !{!2251, !700, !700, !700}
!2251 = !DIDerivedType(tag: DW_TAG_typedef, name: "type", scope: !2253, file: !2252, line: 21, baseType: !1070)
!2252 = !DIFile(filename: "enable_if.h", directory: "")
!2253 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "enable_if<true, std::__h::pair<std::__h::reverse_iterator<char **>, std::__h::reverse_iterator<char **> > >", scope: !15, file: !2252, line: 21, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !2254, identifier: "_ZTSNSt3__h9enable_ifILb1ENS_4pairINS_16reverse_iteratorIPPcEES5_EEEE")
!2254 = !{!1069, !2255}
!2255 = !DITemplateTypeParameter(name: "_Tp", type: !1070)
!2256 = !{!2247, !2257, !2258, !2259}
!2257 = !DILocalVariable(name: "__last", arg: 2, scope: !2248, file: !2234, line: 96, type: !700)
!2258 = !DILocalVariable(name: "__result", arg: 3, scope: !2248, file: !2234, line: 96, type: !700)
!2259 = !DILocalVariable(name: "__ret", scope: !2248, file: !2234, line: 97, type: !1070)
!2260 = !{!2261, !2264, !2265, !2266}
!2261 = !DITemplateTypeParameter(name: "_AlgPolicy", type: !2262)
!2262 = !DICompositeType(tag: DW_TAG_structure_type, name: "_ClassicAlgPolicy", scope: !15, file: !2263, line: 63, size: 8, flags: DIFlagFwdDecl, identifier: "_ZTSNSt3__h17_ClassicAlgPolicyE")
!2263 = !DIFile(filename: "iterator_operations.h", directory: "")
!2264 = !DITemplateTypeParameter(name: "_InIter", type: !700)
!2265 = !DITemplateTypeParameter(name: "_Sent", type: !700)
!2266 = !DITemplateTypeParameter(name: "_OutIter", type: !700)
!2267 = !DILocation(line: 0, scope: !2248, inlinedAt: !2268)
!2268 = distinct !DILocation(line: 114, column: 10, scope: !2233, inlinedAt: !2244)
!2269 = !DILocalVariable(name: "__first", arg: 1, scope: !2270, file: !2234, line: 80, type: !700)
!2270 = distinct !DISubprogram(name: "__move_impl<std::__h::_ClassicAlgPolicy, char **, char **, 0>", linkageName: "_ZNSt3__h11__move_implB6v15004INS_17_ClassicAlgPolicyEPPcS3_Li0EEENS_4pairINS_16reverse_iteratorIT0_EENS5_IT1_EEEES7_S7_S9_", scope: !15, file: !2234, line: 80, type: !2271, scopeLine: 82, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2291, retainedNodes: !2273)
!2271 = !DISubroutineType(types: !2272)
!2272 = !{!1070, !700, !700, !700}
!2273 = !{!2269, !2274, !2275, !2276, !2288, !2289, !2290}
!2274 = !DILocalVariable(name: "__last", arg: 2, scope: !2270, file: !2234, line: 81, type: !700)
!2275 = !DILocalVariable(name: "__result", arg: 3, scope: !2270, file: !2234, line: 82, type: !700)
!2276 = !DILocalVariable(name: "__first_base", scope: !2270, file: !2234, line: 83, type: !2277)
!2277 = !DIDerivedType(tag: DW_TAG_typedef, name: "_ToAddressT", scope: !2279, file: !2278, line: 44, baseType: !67)
!2278 = !DIFile(filename: "unwrap_iter.h", directory: "")
!2279 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__unwrap_iter_impl<char **, true>", scope: !15, file: !2278, line: 43, size: 8, flags: DIFlagTypePassByValue, elements: !2280, templateParams: !2287, identifier: "_ZTSNSt3__h18__unwrap_iter_implIPPcLb1EEE")
!2280 = !{!2281, !2284}
!2281 = !DISubprogram(name: "__rewrap", linkageName: "_ZNSt3__h18__unwrap_iter_implIPPcLb1EE8__rewrapB6v15004ES2_S2_", scope: !2279, file: !2278, line: 46, type: !2282, scopeLine: 46, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2282 = !DISubroutineType(types: !2283)
!2283 = !{!67, !67, !2277}
!2284 = !DISubprogram(name: "__unwrap", linkageName: "_ZNSt3__h18__unwrap_iter_implIPPcLb1EE8__unwrapB6v15004ES2_", scope: !2279, file: !2278, line: 50, type: !2285, scopeLine: 50, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2285 = !DISubroutineType(types: !2286)
!2286 = !{!2277, !67}
!2287 = !{!743, !1069}
!2288 = !DILocalVariable(name: "__last_base", scope: !2270, file: !2234, line: 84, type: !2277)
!2289 = !DILocalVariable(name: "__result_base", scope: !2270, file: !2234, line: 85, type: !2277)
!2290 = !DILocalVariable(name: "__result_first", scope: !2270, file: !2234, line: 86, type: !2277)
!2291 = !{!2261, !2292, !2293, !2294}
!2292 = !DITemplateTypeParameter(name: "_InIter", type: !67)
!2293 = !DITemplateTypeParameter(name: "_OutIter", type: !67)
!2294 = !DITemplateValueParameter(type: !17, value: i32 0)
!2295 = !DILocation(line: 0, scope: !2270, inlinedAt: !2296)
!2296 = distinct !DILocation(line: 97, column: 16, scope: !2248, inlinedAt: !2268)
!2297 = !DILocalVariable(name: "__first", arg: 1, scope: !2298, file: !2234, line: 47, type: !67)
!2298 = distinct !DISubprogram(name: "__move_impl<std::__h::_ClassicAlgPolicy, char *, char *, void>", linkageName: "_ZNSt3__h11__move_implB6v15004INS_17_ClassicAlgPolicyEPcS2_vEENS_4pairIPT0_PT1_EES5_S5_S7_", scope: !15, file: !2234, line: 47, type: !2299, scopeLine: 47, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2306, retainedNodes: !2301)
!2299 = !DISubroutineType(types: !2300)
!2300 = !{!1110, !67, !67, !67}
!2301 = !{!2297, !2302, !2303, !2304}
!2302 = !DILocalVariable(name: "__last", arg: 2, scope: !2298, file: !2234, line: 47, type: !67)
!2303 = !DILocalVariable(name: "__result", arg: 3, scope: !2298, file: !2234, line: 47, type: !67)
!2304 = !DILocalVariable(name: "__n", scope: !2298, file: !2234, line: 55, type: !2305)
!2305 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !68)
!2306 = !{!2261, !2307, !2308, !2196}
!2307 = !DITemplateTypeParameter(name: "_InType", type: !30)
!2308 = !DITemplateTypeParameter(name: "_OutType", type: !30)
!2309 = !DILocation(line: 0, scope: !2298, inlinedAt: !2310)
!2310 = distinct !DILocation(line: 87, column: 3, scope: !2270, inlinedAt: !2296)
!2311 = !DILocalVariable(name: "__x", arg: 1, scope: !2312, file: !2313, line: 33, type: !134)
!2312 = distinct !DISubprogram(name: "swap<char **>", linkageName: "_ZNSt3__h4swapB6v15004IPPcEENS_9enable_ifIXaasr21is_move_constructibleIT_EE5valuesr18is_move_assignableIS4_EE5valueEvE4typeERS4_S7_", scope: !15, file: !2313, line: 33, type: !2314, scopeLine: 34, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2324, retainedNodes: !2321)
!2313 = !DIFile(filename: "swap.h", directory: "")
!2314 = !DISubroutineType(types: !2315)
!2315 = !{!2316, !134, !134}
!2316 = !DIDerivedType(tag: DW_TAG_typedef, name: "__swap_result_t<char **>", scope: !15, file: !2313, line: 26, baseType: !2317)
!2317 = !DIDerivedType(tag: DW_TAG_typedef, name: "type", scope: !2318, file: !2252, line: 21, baseType: null)
!2318 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "enable_if<true, void>", scope: !15, file: !2252, line: 21, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !2319, identifier: "_ZTSNSt3__h9enable_ifILb1EvEE")
!2319 = !{!1069, !2320}
!2320 = !DITemplateTypeParameter(name: "_Tp", type: null, defaulted: true)
!2321 = !{!2311, !2322, !2323}
!2322 = !DILocalVariable(name: "__y", arg: 2, scope: !2312, file: !2313, line: 33, type: !134)
!2323 = !DILocalVariable(name: "__t", scope: !2312, file: !2313, line: 35, type: !67)
!2324 = !{!144}
!2325 = !DILocation(line: 0, scope: !2312, inlinedAt: !2326)
!2326 = distinct !DILocation(line: 927, column: 5, scope: !695, inlinedAt: !2216)
!2327 = !DILocation(line: 36, column: 7, scope: !2312, inlinedAt: !2326)
!2328 = !{!"__x", !"char*"}
!2329 = !DILocation(line: 0, scope: !2312, inlinedAt: !2330)
!2330 = distinct !DILocation(line: 928, column: 5, scope: !695, inlinedAt: !2216)
!2331 = !DILocation(line: 0, scope: !2312, inlinedAt: !2332)
!2332 = distinct !DILocation(line: 929, column: 5, scope: !695, inlinedAt: !2216)
!2333 = !DILocalVariable(name: "this", arg: 1, scope: !2334, type: !2146, flags: DIFlagArtificial | DIFlagObjectPointer)
!2334 = distinct !DISubprogram(name: "~__split_buffer", linkageName: "_ZNSt3__h14__split_bufferIPcRNS_9allocatorIS1_EEED2Ev", scope: !403, file: !404, line: 351, type: !496, scopeLine: 352, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !508, retainedNodes: !2335)
!2335 = !{!2333}
!2336 = !DILocation(line: 0, scope: !2334, inlinedAt: !2337)
!2337 = distinct !DILocation(line: 1574, column: 1, scope: !2117, inlinedAt: !2129)
!2338 = !DILocation(line: 38, column: 26, scope: !2091)
!2339 = !DILocation(line: 0, scope: !2105, inlinedAt: !2340)
!2340 = distinct !DILocation(line: 38, column: 9, scope: !2091)
!2341 = !DILocation(line: 0, scope: !2111, inlinedAt: !2342)
!2342 = distinct !DILocation(line: 1596, column: 30, scope: !2115, inlinedAt: !2340)
!2343 = !DILocation(line: 0, scope: !2117, inlinedAt: !2344)
!2344 = distinct !DILocation(line: 1601, column: 9, scope: !2115, inlinedAt: !2340)
!2345 = !DILocation(line: 0, scope: !2131, inlinedAt: !2346)
!2346 = distinct !DILocation(line: 1569, column: 49, scope: !2117, inlinedAt: !2344)
!2347 = !DILocation(line: 0, scope: !2150, inlinedAt: !2348)
!2348 = distinct !DILocation(line: 316, column: 29, scope: !2137, inlinedAt: !2346)
!2349 = !DILocation(line: 0, scope: !2158, inlinedAt: !2350)
!2350 = distinct !DILocation(line: 54, column: 19, scope: !2150, inlinedAt: !2348)
!2351 = !DILocation(line: 0, scope: !2165, inlinedAt: !2352)
!2352 = distinct !DILocation(line: 112, column: 38, scope: !2173, inlinedAt: !2350)
!2353 = !DILocation(line: 0, scope: !2176, inlinedAt: !2354)
!2354 = distinct !DILocation(line: 272, column: 10, scope: !2165, inlinedAt: !2352)
!2355 = !DILocation(line: 246, column: 10, scope: !2176, inlinedAt: !2354)
!2356 = !DILocation(line: 320, column: 34, scope: !2139, inlinedAt: !2346)
!2357 = !DILocation(line: 0, scope: !2188, inlinedAt: !2358)
!2358 = distinct !DILocation(line: 1571, column: 5, scope: !2117, inlinedAt: !2344)
!2359 = !DILocation(line: 0, scope: !2203, inlinedAt: !2360)
!2360 = distinct !DILocation(line: 290, column: 13, scope: !2188, inlinedAt: !2358)
!2361 = !DILocation(line: 165, column: 9, scope: !2203, inlinedAt: !2360)
!2362 = !DILocation(line: 0, scope: !695, inlinedAt: !2363)
!2363 = distinct !DILocation(line: 1573, column: 5, scope: !2117, inlinedAt: !2344)
!2364 = !DILocation(line: 0, scope: !2218, inlinedAt: !2365)
!2365 = distinct !DILocation(line: 924, column: 22, scope: !695, inlinedAt: !2363)
!2366 = !DILocation(line: 0, scope: !2233, inlinedAt: !2367)
!2367 = distinct !DILocation(line: 635, column: 12, scope: !2245, inlinedAt: !2365)
!2368 = !DILocation(line: 0, scope: !2248, inlinedAt: !2369)
!2369 = distinct !DILocation(line: 114, column: 10, scope: !2233, inlinedAt: !2367)
!2370 = !DILocation(line: 0, scope: !2270, inlinedAt: !2371)
!2371 = distinct !DILocation(line: 97, column: 16, scope: !2248, inlinedAt: !2369)
!2372 = !DILocation(line: 0, scope: !2298, inlinedAt: !2373)
!2373 = distinct !DILocation(line: 87, column: 3, scope: !2270, inlinedAt: !2371)
!2374 = !DILocation(line: 56, column: 3, scope: !2298, inlinedAt: !2373)
!2375 = !DILocation(line: 0, scope: !2312, inlinedAt: !2376)
!2376 = distinct !DILocation(line: 927, column: 5, scope: !695, inlinedAt: !2363)
!2377 = !DILocation(line: 0, scope: !2312, inlinedAt: !2378)
!2378 = distinct !DILocation(line: 928, column: 5, scope: !695, inlinedAt: !2363)
!2379 = !DILocation(line: 0, scope: !2312, inlinedAt: !2380)
!2380 = distinct !DILocation(line: 929, column: 5, scope: !695, inlinedAt: !2363)
!2381 = !DILocation(line: 0, scope: !2334, inlinedAt: !2382)
!2382 = distinct !DILocation(line: 1574, column: 1, scope: !2117, inlinedAt: !2344)
!2383 = !DILocalVariable(name: "__a", arg: 1, scope: !2384, file: !38, line: 281, type: !44)
!2384 = distinct !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h16allocator_traitsINS_9allocatorIPcEEE10deallocateB6v15004ERS3_PS2_m", scope: !39, file: !38, line: 281, type: !107, scopeLine: 281, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !106, retainedNodes: !2385)
!2385 = !{!2383, !2386, !2387}
!2386 = !DILocalVariable(name: "__p", arg: 2, scope: !2384, file: !38, line: 281, type: !37)
!2387 = !DILocalVariable(name: "__n", arg: 3, scope: !2384, file: !38, line: 281, type: !104)
!2388 = !DILocation(line: 0, scope: !2384, inlinedAt: !2389)
!2389 = distinct !DILocation(line: 355, column: 9, scope: !2390, inlinedAt: !2382)
!2390 = distinct !DILexicalBlock(scope: !2391, file: !404, line: 354, column: 9)
!2391 = distinct !DILexicalBlock(scope: !2334, file: !404, line: 352, column: 1)
!2392 = !DILocalVariable(name: "this", arg: 1, scope: !2393, type: !2161, flags: DIFlagArtificial | DIFlagObjectPointer)
!2393 = distinct !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h9allocatorIPcE10deallocateB6v15004EPS1_m", scope: !46, file: !47, line: 124, type: !72, scopeLine: 124, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !71, retainedNodes: !2394)
!2394 = !{!2392, !2395, !2396}
!2395 = !DILocalVariable(name: "__p", arg: 2, scope: !2393, file: !47, line: 124, type: !67)
!2396 = !DILocalVariable(name: "__n", arg: 3, scope: !2393, file: !47, line: 124, type: !68)
!2397 = !DILocation(line: 0, scope: !2393, inlinedAt: !2398)
!2398 = distinct !DILocation(line: 282, column: 13, scope: !2384, inlinedAt: !2389)
!2399 = !DILocalVariable(name: "__ptr", arg: 1, scope: !2400, file: !2166, line: 287, type: !691)
!2400 = distinct !DISubprogram(name: "__libcpp_deallocate", linkageName: "_ZNSt3__h19__libcpp_deallocateB6v15004EPvmm", scope: !15, file: !2166, line: 287, type: !2401, scopeLine: 287, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2403)
!2401 = !DISubroutineType(types: !2402)
!2402 = !{null, !691, !68, !68}
!2403 = !{!2399, !2404, !2405}
!2404 = !DILocalVariable(name: "__size", arg: 2, scope: !2400, file: !2166, line: 287, type: !68)
!2405 = !DILocalVariable(name: "__align", arg: 3, scope: !2400, file: !2166, line: 287, type: !68)
!2406 = !DILocation(line: 0, scope: !2400, inlinedAt: !2407)
!2407 = distinct !DILocation(line: 128, column: 13, scope: !2408, inlinedAt: !2398)
!2408 = distinct !DILexicalBlock(scope: !2409, file: !47, line: 127, column: 16)
!2409 = distinct !DILexicalBlock(scope: !2393, file: !47, line: 125, column: 13)
!2410 = !DILocalVariable(name: "__ptr", arg: 1, scope: !2411, file: !2166, line: 277, type: !691)
!2411 = distinct !DISubprogram(name: "__do_deallocate_handle_size<>", linkageName: "_ZNSt3__h27__do_deallocate_handle_sizeB6v15004IJEEEvPvmDpT_", scope: !15, file: !2166, line: 277, type: !2412, scopeLine: 277, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2416, retainedNodes: !2414)
!2412 = !DISubroutineType(types: !2413)
!2413 = !{null, !691, !68}
!2414 = !{!2410, !2415}
!2415 = !DILocalVariable(name: "__size", arg: 2, scope: !2411, file: !2166, line: 277, type: !68)
!2416 = !{!2417}
!2417 = !DITemplateValueParameter(tag: DW_TAG_GNU_template_parameter_pack, name: "_Args", value: !125)
!2418 = !DILocation(line: 0, scope: !2411, inlinedAt: !2419)
!2419 = distinct !DILocation(line: 290, column: 12, scope: !2400, inlinedAt: !2407)
!2420 = !DILocalVariable(name: "__args", arg: 1, scope: !2421, file: !2166, line: 254, type: !691)
!2421 = distinct !DISubprogram(name: "__libcpp_operator_delete<void *>", linkageName: "_ZNSt3__h24__libcpp_operator_deleteB6v15004IJPvEEEvDpT_", scope: !15, file: !2166, line: 254, type: !797, scopeLine: 254, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2423, retainedNodes: !2422)
!2422 = !{!2420}
!2423 = !{!2424}
!2424 = !DITemplateValueParameter(tag: DW_TAG_GNU_template_parameter_pack, name: "_Args", value: !2425)
!2425 = !{!2426}
!2426 = !DITemplateTypeParameter(type: !691)
!2427 = !DILocation(line: 0, scope: !2421, inlinedAt: !2428)
!2428 = distinct !DILocation(line: 280, column: 10, scope: !2411, inlinedAt: !2419)
!2429 = !DILocation(line: 256, column: 3, scope: !2421, inlinedAt: !2428)
!2430 = !DILocalVariable(name: "this", arg: 1, scope: !2431, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2431 = distinct !DISubprogram(name: "operator[]", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEEixB6v15004Em", scope: !33, file: !32, line: 1488, type: !321, scopeLine: 1489, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !320, retainedNodes: !2432)
!2432 = !{!2430, !2433}
!2433 = !DILocalVariable(name: "__n", arg: 2, scope: !2431, file: !32, line: 558, type: !221)
!2434 = !DILocation(line: 0, scope: !2431, inlinedAt: !2435)
!2435 = distinct !DILocation(line: 39, column: 14, scope: !2091)
!2436 = !DILocation(line: 39, column: 14, scope: !2091)
!2437 = !DILocation(line: 0, scope: !2057, inlinedAt: !2438)
!2438 = distinct !DILocation(line: 39, column: 5, scope: !2091)
!2439 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2438)
!2440 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2438)
!2441 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2438)
!2442 = !DILocation(line: 0, scope: !2431, inlinedAt: !2443)
!2443 = distinct !DILocation(line: 39, column: 32, scope: !2091)
!2444 = !DILocation(line: 1491, column: 12, scope: !2431, inlinedAt: !2443)
!2445 = !DILocation(line: 39, column: 32, scope: !2091)
!2446 = !DILocation(line: 0, scope: !2057, inlinedAt: !2447)
!2447 = distinct !DILocation(line: 39, column: 23, scope: !2091)
!2448 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2447)
!2449 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2447)
!2450 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2447)
!2451 = !DILocalVariable(name: "this", arg: 1, scope: !2452, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2452 = distinct !DISubprogram(name: "~vector", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEED2B6v15004Ev", scope: !33, file: !32, line: 449, type: !209, scopeLine: 449, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !231, retainedNodes: !2453)
!2453 = !{!2451}
!2454 = !DILocation(line: 0, scope: !2452, inlinedAt: !2455)
!2455 = distinct !DILocation(line: 40, column: 1, scope: !2091)
!2456 = !DILocalVariable(name: "this", arg: 1, scope: !2457, type: !2459, flags: DIFlagArtificial | DIFlagObjectPointer)
!2457 = distinct !DISubprogram(name: "operator()", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE16__destroy_vectorclB6v15004Ev", scope: !31, file: !32, line: 434, type: !689, scopeLine: 434, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !688, retainedNodes: !2458)
!2458 = !{!2456}
!2459 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64)
!2460 = !DILocation(line: 0, scope: !2457, inlinedAt: !2461)
!2461 = distinct !DILocation(line: 449, column: 67, scope: !2462, inlinedAt: !2455)
!2462 = distinct !DILexicalBlock(scope: !2452, file: !32, line: 449, column: 65)
!2463 = !DILocalVariable(name: "this", arg: 1, scope: !2464, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2464 = distinct !DISubprogram(name: "__clear", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE7__clearB6v15004Ev", scope: !33, file: !32, line: 837, type: !209, scopeLine: 837, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !666, retainedNodes: !2465)
!2465 = !{!2463}
!2466 = !DILocation(line: 0, scope: !2464, inlinedAt: !2467)
!2467 = distinct !DILocation(line: 439, column: 20, scope: !2468, inlinedAt: !2461)
!2468 = distinct !DILexicalBlock(scope: !2469, file: !32, line: 438, column: 43)
!2469 = distinct !DILexicalBlock(scope: !2457, file: !32, line: 438, column: 15)
!2470 = !DILocalVariable(name: "this", arg: 1, scope: !2471, type: !698, flags: DIFlagArtificial | DIFlagObjectPointer)
!2471 = distinct !DISubprogram(name: "__base_destruct_at_end", linkageName: "_ZNSt3__h6vectorIPcNS_9allocatorIS1_EEE22__base_destruct_at_endB6v15004EPS1_", scope: !33, file: !32, line: 840, type: !380, scopeLine: 840, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !667, retainedNodes: !2472)
!2472 = !{!2470, !2473, !2474}
!2473 = !DILocalVariable(name: "__new_last", arg: 2, scope: !2471, file: !32, line: 840, type: !36)
!2474 = !DILocalVariable(name: "__soon_to_be_end", scope: !2471, file: !32, line: 841, type: !36)
!2475 = !DILocation(line: 0, scope: !2471, inlinedAt: !2476)
!2476 = distinct !DILocation(line: 837, column: 29, scope: !2464, inlinedAt: !2467)
!2477 = !DILocation(line: 0, scope: !2384, inlinedAt: !2478)
!2478 = distinct !DILocation(line: 440, column: 13, scope: !2468, inlinedAt: !2461)
!2479 = !DILocation(line: 0, scope: !2393, inlinedAt: !2480)
!2480 = distinct !DILocation(line: 282, column: 13, scope: !2384, inlinedAt: !2478)
!2481 = !DILocation(line: 0, scope: !2400, inlinedAt: !2482)
!2482 = distinct !DILocation(line: 128, column: 13, scope: !2408, inlinedAt: !2480)
!2483 = !DILocation(line: 0, scope: !2411, inlinedAt: !2484)
!2484 = distinct !DILocation(line: 290, column: 12, scope: !2400, inlinedAt: !2482)
!2485 = !DILocation(line: 0, scope: !2421, inlinedAt: !2486)
!2486 = distinct !DILocation(line: 280, column: 10, scope: !2411, inlinedAt: !2484)
!2487 = !DILocation(line: 256, column: 3, scope: !2421, inlinedAt: !2486)
!2488 = !DILocation(line: 40, column: 1, scope: !2091)
!2489 = !DILocation(line: 438, column: 22, scope: !2469, inlinedAt: !2490)
!2490 = distinct !DILocation(line: 449, column: 67, scope: !2462, inlinedAt: !2491)
!2491 = distinct !DILocation(line: 40, column: 1, scope: !2091)
!2492 = !{!2493, !2069, i64 0}
!2493 = !{!"_ZTSNSt3__h6vectorIPcNS_9allocatorIS1_EEEE", !2069, i64 0, !2069, i64 8, !2494, i64 16}
!2494 = !{!"_ZTSNSt3__h17__compressed_pairIPPcNS_9allocatorIS1_EEEE", !2495, i64 0}
!2495 = !{!"_ZTSNSt3__h22__compressed_pair_elemIPPcLi0ELb0EEE", !2069, i64 0}
!2496 = !DILocation(line: 0, scope: !2452, inlinedAt: !2491)
!2497 = !DILocation(line: 0, scope: !2457, inlinedAt: !2490)
!2498 = !DILocation(line: 438, column: 31, scope: !2469, inlinedAt: !2490)
!2499 = !DILocation(line: 438, column: 15, scope: !2457, inlinedAt: !2490)
!2500 = !DILocation(line: 0, scope: !2464, inlinedAt: !2501)
!2501 = distinct !DILocation(line: 439, column: 20, scope: !2468, inlinedAt: !2490)
!2502 = !DILocation(line: 0, scope: !2471, inlinedAt: !2503)
!2503 = distinct !DILocation(line: 837, column: 29, scope: !2464, inlinedAt: !2501)
!2504 = !DILocation(line: 0, scope: !2384, inlinedAt: !2505)
!2505 = distinct !DILocation(line: 440, column: 13, scope: !2468, inlinedAt: !2490)
!2506 = !DILocation(line: 0, scope: !2393, inlinedAt: !2507)
!2507 = distinct !DILocation(line: 282, column: 13, scope: !2384, inlinedAt: !2505)
!2508 = !DILocation(line: 0, scope: !2400, inlinedAt: !2509)
!2509 = distinct !DILocation(line: 128, column: 13, scope: !2408, inlinedAt: !2507)
!2510 = !DILocation(line: 0, scope: !2411, inlinedAt: !2511)
!2511 = distinct !DILocation(line: 290, column: 12, scope: !2400, inlinedAt: !2509)
!2512 = !DILocation(line: 0, scope: !2421, inlinedAt: !2513)
!2513 = distinct !DILocation(line: 280, column: 10, scope: !2411, inlinedAt: !2511)
!2514 = !DILocation(line: 256, column: 3, scope: !2421, inlinedAt: !2513)
!2515 = !DILocation(line: 441, column: 11, scope: !2468, inlinedAt: !2490)
!2516 = distinct !DISubprogram(name: "test_smart_ptr_store", linkageName: "_Z20test_smart_ptr_storev", scope: !2, file: !2, line: 43, type: !1447, scopeLine: 43, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2517)
!2517 = !{!2518, !2519}
!2518 = !DILocalVariable(name: "ptr1", scope: !2516, file: !2, line: 44, type: !821)
!2519 = !DILocalVariable(name: "ptr2", scope: !2516, file: !2, line: 45, type: !913)
!2520 = !DILocation(line: 44, column: 56, scope: !2516)
!2521 = !DILocation(line: 0, scope: !2516)
!2522 = !DILocation(line: 45, column: 39, scope: !2516)
!2523 = !DILocalVariable(name: "this", arg: 1, scope: !2524, type: !2533, flags: DIFlagArtificial | DIFlagObjectPointer)
!2524 = distinct !DISubprogram(name: "shared_ptr<char, void (*)(void *), void>", linkageName: "_ZNSt3__h10shared_ptrIcEC2B6v15004IcPFvPvEvEEPT_T0_", scope: !913, file: !914, line: 467, type: !2525, scopeLine: 469, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2528, declaration: !2527, retainedNodes: !2530)
!2525 = !DISubroutineType(types: !2526)
!2526 = !{null, !925, !30, !796}
!2527 = !DISubprogram(name: "shared_ptr<char, void (*)(void *), void>", scope: !913, file: !914, line: 467, type: !2525, scopeLine: 467, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized, templateParams: !2528)
!2528 = !{!2529, !912, !2196}
!2529 = !DITemplateTypeParameter(name: "_Yp", type: !5)
!2530 = !{!2523, !2531, !2532}
!2531 = !DILocalVariable(name: "__p", arg: 2, scope: !2524, file: !914, line: 467, type: !30)
!2532 = !DILocalVariable(name: "__d", arg: 3, scope: !2524, file: !914, line: 467, type: !796)
!2533 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !913, size: 64)
!2534 = !DILocation(line: 0, scope: !2524, inlinedAt: !2535)
!2535 = distinct !DILocation(line: 45, column: 27, scope: !2516)
!2536 = !DILocation(line: 477, column: 24, scope: !2537, inlinedAt: !2535)
!2537 = distinct !DILexicalBlock(scope: !2538, file: !914, line: 472, column: 9)
!2538 = distinct !DILexicalBlock(scope: !2524, file: !914, line: 469, column: 5)
!2539 = !{!"this->__cntrl_", !"__shared_weak_count*"}
!2540 = !DIDerivedType(tag: DW_TAG_typedef, name: "_CntrlBlk", scope: !2524, file: !914, line: 475, baseType: !1237)
!2541 = !DILocation(line: 490, column: 5, scope: !2537, inlinedAt: !2535)
!2542 = !DILocation(line: 483, column: 9, scope: !2537, inlinedAt: !2535)
!2543 = !DILocation(line: 486, column: 13, scope: !2544, inlinedAt: !2535)
!2544 = distinct !DILexicalBlock(scope: !2538, file: !914, line: 485, column: 9)
!2545 = !DILocation(line: 487, column: 13, scope: !2544, inlinedAt: !2535)
!2546 = !DILocation(line: 490, column: 5, scope: !2544, inlinedAt: !2535)
!2547 = !DILocation(line: 488, column: 9, scope: !2544, inlinedAt: !2535)
!2548 = !DILocalVariable(name: "__a", arg: 4, scope: !2549, file: !914, line: 236, type: !1151)
!2549 = distinct !DISubprogram(name: "__shared_ptr_pointer", linkageName: "_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEEC2B6v15004ES1_S4_S6_", scope: !1237, file: !914, line: 236, type: !1277, scopeLine: 237, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !1276, retainedNodes: !2550)
!2550 = !{!2551, !2552, !2553, !2548}
!2551 = !DILocalVariable(name: "this", arg: 1, scope: !2549, type: !1314, flags: DIFlagArtificial | DIFlagObjectPointer)
!2552 = !DILocalVariable(name: "__p", arg: 2, scope: !2549, file: !914, line: 236, type: !30)
!2553 = !DILocalVariable(name: "__d", arg: 3, scope: !2549, file: !914, line: 236, type: !796)
!2554 = !DILocation(line: 0, scope: !2549, inlinedAt: !2555)
!2555 = distinct !DILocation(line: 477, column: 28, scope: !2537, inlinedAt: !2535)
!2556 = !DILocalVariable(name: "this", arg: 1, scope: !2557, type: !920, flags: DIFlagArtificial | DIFlagObjectPointer)
!2557 = distinct !DISubprogram(name: "__shared_weak_count", linkageName: "_ZNSt3__h19__shared_weak_countC2B6v15004El", scope: !921, file: !914, line: 193, type: !2558, scopeLine: 195, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2561, retainedNodes: !2562)
!2558 = !DISubroutineType(types: !2559)
!2559 = !{null, !2560, !719}
!2560 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !921, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!2561 = !DISubprogram(name: "__shared_weak_count", scope: !921, file: !914, line: 193, type: !2558, scopeLine: 193, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2562 = !{!2556, !2563}
!2563 = !DILocalVariable(name: "__refs", arg: 2, scope: !2557, file: !914, line: 193, type: !719)
!2564 = !DILocation(line: 0, scope: !2557, inlinedAt: !2565)
!2565 = distinct !DILocation(line: 236, column: 5, scope: !2549, inlinedAt: !2555)
!2566 = !DILocalVariable(name: "this", arg: 1, scope: !2567, type: !2574, flags: DIFlagArtificial | DIFlagObjectPointer)
!2567 = distinct !DISubprogram(name: "__shared_count", linkageName: "_ZNSt3__h14__shared_countC2B6v15004El", scope: !1293, file: !914, line: 160, type: !2568, scopeLine: 161, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2571, retainedNodes: !2572)
!2568 = !DISubroutineType(types: !2569)
!2569 = !{null, !2570, !719}
!2570 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1293, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!2571 = !DISubprogram(name: "__shared_count", scope: !1293, file: !914, line: 160, type: !2568, scopeLine: 160, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2572 = !{!2566, !2573}
!2573 = !DILocalVariable(name: "__refs", arg: 2, scope: !2567, file: !914, line: 160, type: !719)
!2574 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1293, size: 64)
!2575 = !DILocation(line: 0, scope: !2567, inlinedAt: !2576)
!2576 = distinct !DILocation(line: 194, column: 11, scope: !2557, inlinedAt: !2565)
!2577 = !DILocation(line: 161, column: 11, scope: !2567, inlinedAt: !2576)
!2578 = !DILocation(line: 195, column: 11, scope: !2557, inlinedAt: !2565)
!2579 = !DILocation(line: 237, column: 90, scope: !2549, inlinedAt: !2555)
!2580 = !{!2581, !2581, i64 0}
!2581 = !{!"vtable pointer", !2065, i64 0}
!2582 = !{!"this", !"__shared_ptr_pointer<char *, void (*)(void *), std::__h::allocator<char> >"}
!2583 = !DILocation(line: 237, column: 12, scope: !2549, inlinedAt: !2555)
!2584 = !DILocalVariable(name: "this", arg: 1, scope: !2585, type: !1264, flags: DIFlagArtificial | DIFlagObjectPointer)
!2585 = distinct !DISubprogram(name: "__compressed_pair<std::__h::__compressed_pair<char *, void (*)(void *)>, std::__h::allocator<char> >", linkageName: "_ZNSt3__h17__compressed_pairINS0_IPcPFvPvEEENS_9allocatorIcEEEC2B6v15004IS5_S7_EEOT_OT0_", scope: !1241, file: !114, line: 108, type: !2586, scopeLine: 108, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2591, declaration: !2590, retainedNodes: !2594)
!2586 = !DISubroutineType(types: !2587)
!2587 = !{null, !1248, !2588, !2589}
!2588 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !825, size: 64)
!2589 = !DIDerivedType(tag: DW_TAG_rvalue_reference_type, baseType: !1151, size: 64)
!2590 = !DISubprogram(name: "__compressed_pair<std::__h::__compressed_pair<char *, void (*)(void *)>, std::__h::allocator<char> >", scope: !1241, file: !114, line: 108, type: !2586, scopeLine: 108, flags: DIFlagPublic | DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized, templateParams: !2591)
!2591 = !{!2592, !2593}
!2592 = !DITemplateTypeParameter(name: "_U1", type: !825)
!2593 = !DITemplateTypeParameter(name: "_U2", type: !1151)
!2594 = !{!2584, !2595, !2596}
!2595 = !DILocalVariable(name: "__t1", arg: 2, scope: !2585, file: !114, line: 108, type: !2588)
!2596 = !DILocalVariable(name: "__t2", arg: 3, scope: !2585, file: !114, line: 108, type: !2589)
!2597 = !DILocation(line: 0, scope: !2585, inlinedAt: !2598)
!2598 = distinct !DILocation(line: 237, column: 12, scope: !2549, inlinedAt: !2555)
!2599 = !DILocalVariable(name: "this", arg: 1, scope: !2600, type: !1263, flags: DIFlagArtificial | DIFlagObjectPointer)
!2600 = distinct !DISubprogram(name: "__compressed_pair_elem<std::__h::__compressed_pair<char *, void (*)(void *)>, void>", linkageName: "_ZNSt3__h22__compressed_pair_elemINS_17__compressed_pairIPcPFvPvEEELi0ELb0EEC2B6v15004IS6_vEEOT_", scope: !1187, file: !114, line: 40, type: !2601, scopeLine: 40, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2604, declaration: !2603, retainedNodes: !2606)
!2601 = !DISubroutineType(types: !2602)
!2602 = !{null, !1193, !2588}
!2603 = !DISubprogram(name: "__compressed_pair_elem<std::__h::__compressed_pair<char *, void (*)(void *)>, void>", scope: !1187, file: !114, line: 40, type: !2601, scopeLine: 40, flags: DIFlagExplicit | DIFlagPrototyped, spFlags: DISPFlagOptimized, templateParams: !2604)
!2604 = !{!2605, !2196}
!2605 = !DITemplateTypeParameter(name: "_Up", type: !825)
!2606 = !{!2599, !2607}
!2607 = !DILocalVariable(name: "__u", arg: 2, scope: !2600, file: !114, line: 40, type: !2588)
!2608 = !DILocation(line: 0, scope: !2600, inlinedAt: !2609)
!2609 = distinct !DILocation(line: 108, column: 56, scope: !2585, inlinedAt: !2598)
!2610 = !DILocation(line: 40, column: 48, scope: !2600, inlinedAt: !2609)
!2611 = !DILocation(line: 0, scope: !2057, inlinedAt: !2612)
!2612 = distinct !DILocation(line: 46, column: 5, scope: !2516)
!2613 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2612)
!2614 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2612)
!2615 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2612)
!2616 = !DILocation(line: 0, scope: !2057, inlinedAt: !2617)
!2617 = distinct !DILocation(line: 46, column: 27, scope: !2516)
!2618 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2617)
!2619 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2617)
!2620 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2617)
!2621 = !DILocalVariable(name: "this", arg: 1, scope: !2622, type: !2533, flags: DIFlagArtificial | DIFlagObjectPointer)
!2622 = distinct !DISubprogram(name: "~shared_ptr", linkageName: "_ZNSt3__h10shared_ptrIcED2B6v15004Ev", scope: !913, file: !914, line: 699, type: !923, scopeLine: 700, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !938, retainedNodes: !2623)
!2623 = !{!2621}
!2624 = !DILocation(line: 0, scope: !2622, inlinedAt: !2625)
!2625 = distinct !DILocation(line: 47, column: 1, scope: !2516)
!2626 = !DILocalVariable(name: "this", arg: 1, scope: !2627, type: !920, flags: DIFlagArtificial | DIFlagObjectPointer)
!2627 = distinct !DISubprogram(name: "__release_shared", linkageName: "_ZNSt3__h19__shared_weak_count16__release_sharedB6v15004Ev", scope: !921, file: !914, line: 214, type: !2628, scopeLine: 214, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2630, retainedNodes: !2631)
!2628 = !DISubroutineType(types: !2629)
!2629 = !{null, !2560}
!2630 = !DISubprogram(name: "__release_shared", linkageName: "_ZNSt3__h19__shared_weak_count16__release_sharedB6v15004Ev", scope: !921, file: !914, line: 214, type: !2628, scopeLine: 214, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2631 = !{!2626}
!2632 = !DILocation(line: 0, scope: !2627, inlinedAt: !2633)
!2633 = distinct !DILocation(line: 702, column: 23, scope: !2634, inlinedAt: !2625)
!2634 = distinct !DILexicalBlock(scope: !2635, file: !914, line: 701, column: 13)
!2635 = distinct !DILexicalBlock(scope: !2622, file: !914, line: 700, column: 5)
!2636 = !DILocalVariable(name: "this", arg: 1, scope: !2637, type: !2574, flags: DIFlagArtificial | DIFlagObjectPointer)
!2637 = distinct !DISubprogram(name: "__release_shared", linkageName: "_ZNSt3__h14__shared_count16__release_sharedB6v15004Ev", scope: !1293, file: !914, line: 172, type: !2638, scopeLine: 172, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2640, retainedNodes: !2641)
!2638 = !DISubroutineType(types: !2639)
!2639 = !{!58, !2570}
!2640 = !DISubprogram(name: "__release_shared", linkageName: "_ZNSt3__h14__shared_count16__release_sharedB6v15004Ev", scope: !1293, file: !914, line: 172, type: !2638, scopeLine: 172, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2641 = !{!2636}
!2642 = !DILocation(line: 0, scope: !2637, inlinedAt: !2643)
!2643 = distinct !DILocation(line: 215, column: 27, scope: !2644, inlinedAt: !2633)
!2644 = distinct !DILexicalBlock(scope: !2627, file: !914, line: 215, column: 11)
!2645 = !DILocalVariable(name: "__t", arg: 1, scope: !2646, file: !914, line: 116, type: !2649)
!2646 = distinct !DISubprogram(name: "__libcpp_atomic_refcount_decrement<long>", linkageName: "_ZNSt3__h34__libcpp_atomic_refcount_decrementB6v15004IlEET_RS1_", scope: !15, file: !914, line: 116, type: !2647, scopeLine: 117, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, templateParams: !2651, retainedNodes: !2650)
!2647 = !DISubroutineType(types: !2648)
!2648 = !{!719, !2649}
!2649 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !719, size: 64)
!2650 = !{!2645}
!2651 = !{!1068}
!2652 = !DILocation(line: 0, scope: !2646, inlinedAt: !2653)
!2653 = distinct !DILocation(line: 173, column: 11, scope: !2654, inlinedAt: !2643)
!2654 = distinct !DILexicalBlock(scope: !2637, file: !914, line: 173, column: 11)
!2655 = !DILocation(line: 119, column: 12, scope: !2646, inlinedAt: !2653)
!2656 = !DILocation(line: 173, column: 64, scope: !2654, inlinedAt: !2643)
!2657 = !DILocation(line: 173, column: 11, scope: !2637, inlinedAt: !2643)
!2658 = !DILocation(line: 174, column: 9, scope: !2659, inlinedAt: !2643)
!2659 = distinct !DILexicalBlock(scope: !2654, file: !914, line: 173, column: 71)
!2660 = !DILocation(line: 216, column: 9, scope: !2644, inlinedAt: !2633)
!2661 = !DILocalVariable(name: "this", arg: 1, scope: !2662, type: !2664, flags: DIFlagArtificial | DIFlagObjectPointer)
!2662 = distinct !DISubprogram(name: "~unique_ptr", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEED2B6v15004Ev", scope: !821, file: !822, line: 259, type: !870, scopeLine: 259, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !869, retainedNodes: !2663)
!2663 = !{!2661}
!2664 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !821, size: 64)
!2665 = !DILocation(line: 0, scope: !2662, inlinedAt: !2666)
!2666 = distinct !DILocation(line: 47, column: 1, scope: !2516)
!2667 = !DILocalVariable(name: "this", arg: 1, scope: !2668, type: !2664, flags: DIFlagArtificial | DIFlagObjectPointer)
!2668 = distinct !DISubprogram(name: "reset", linkageName: "_ZNSt3__h10unique_ptrIcPFvPvEE5resetB6v15004EPc", scope: !821, file: !822, line: 301, type: !905, scopeLine: 301, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !904, retainedNodes: !2669)
!2669 = !{!2667, !2670, !2671}
!2670 = !DILocalVariable(name: "__p", arg: 2, scope: !2668, file: !822, line: 301, type: !30)
!2671 = !DILocalVariable(name: "__tmp", scope: !2668, file: !822, line: 302, type: !30)
!2672 = !DILocation(line: 0, scope: !2668, inlinedAt: !2673)
!2673 = distinct !DILocation(line: 259, column: 19, scope: !2674, inlinedAt: !2666)
!2674 = distinct !DILexicalBlock(scope: !2662, file: !822, line: 259, column: 17)
!2675 = !DILocation(line: 304, column: 9, scope: !2676, inlinedAt: !2673)
!2676 = distinct !DILexicalBlock(scope: !2668, file: !822, line: 304, column: 9)
!2677 = !DILocation(line: 304, column: 9, scope: !2668, inlinedAt: !2673)
!2678 = !DILocation(line: 305, column: 7, scope: !2676, inlinedAt: !2673)
!2679 = !DILocation(line: 47, column: 1, scope: !2516)
!2680 = !DILocation(line: 0, scope: !2662, inlinedAt: !2681)
!2681 = distinct !DILocation(line: 47, column: 1, scope: !2516)
!2682 = !DILocation(line: 0, scope: !2668, inlinedAt: !2683)
!2683 = distinct !DILocation(line: 259, column: 19, scope: !2674, inlinedAt: !2681)
!2684 = !DILocation(line: 304, column: 9, scope: !2676, inlinedAt: !2683)
!2685 = !DILocation(line: 304, column: 9, scope: !2668, inlinedAt: !2683)
!2686 = !DILocation(line: 305, column: 7, scope: !2676, inlinedAt: !2683)
!2687 = distinct !DISubprogram(name: "test_class_store", linkageName: "_Z16test_class_storev", scope: !2, file: !2, line: 53, type: !1447, scopeLine: 53, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2688)
!2688 = !{!2689, !2691}
!2689 = !DILocalVariable(name: "base", scope: !2687, file: !2, line: 54, type: !2690)
!2690 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !972, size: 64)
!2691 = !DILocalVariable(name: "derived", scope: !2687, file: !2, line: 56, type: !2692)
!2692 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !984, size: 64)
!2693 = !DILocation(line: 54, column: 18, scope: !2687)
!2694 = !{!"base", !"Base*"}
!2695 = !DILocalVariable(name: "this", arg: 1, scope: !2696, type: !2690, flags: DIFlagArtificial | DIFlagObjectPointer)
!2696 = distinct !DISubprogram(name: "Base", linkageName: "_ZN4BaseC2Ev", scope: !972, file: !2, line: 50, type: !981, scopeLine: 50, flags: DIFlagArtificial | DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2697, retainedNodes: !2698)
!2697 = !DISubprogram(name: "Base", scope: !972, type: !981, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2698 = !{!2695}
!2699 = !DILocation(line: 0, scope: !2696, inlinedAt: !2700)
!2700 = distinct !DILocation(line: 54, column: 22, scope: !2687)
!2701 = !DILocation(line: 50, column: 7, scope: !2696, inlinedAt: !2700)
!2702 = !{!"this", !"Base"}
!2703 = !DILocation(line: 0, scope: !2687)
!2704 = !DILocation(line: 55, column: 25, scope: !2687)
!2705 = !{!"base->data", !"char*"}
!2706 = !DILocation(line: 55, column: 11, scope: !2687)
!2707 = !DILocation(line: 55, column: 16, scope: !2687)
!2708 = !{!2709, !2069, i64 8}
!2709 = !{!"_ZTS4Base", !2069, i64 8}
!2710 = !DILocation(line: 56, column: 24, scope: !2687)
!2711 = !{!"derived", !"Derived*"}
!2712 = !DILocation(line: 56, column: 28, scope: !2687)
!2713 = !DILocalVariable(name: "this", arg: 1, scope: !2714, type: !2692, flags: DIFlagArtificial | DIFlagObjectPointer)
!2714 = distinct !DISubprogram(name: "Derived", linkageName: "_ZN7DerivedC2Ev", scope: !984, file: !2, line: 51, type: !2715, scopeLine: 51, flags: DIFlagArtificial | DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2718, retainedNodes: !2719)
!2715 = !DISubroutineType(types: !2716)
!2716 = !{null, !2717}
!2717 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !984, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!2718 = !DISubprogram(name: "Derived", scope: !984, type: !2715, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2719 = !{!2713}
!2720 = !DILocation(line: 0, scope: !2714, inlinedAt: !2721)
!2721 = distinct !DILocation(line: 56, column: 28, scope: !2687)
!2722 = !DILocation(line: 51, column: 7, scope: !2714, inlinedAt: !2721)
!2723 = !{!"this", !"Derived"}
!2724 = !DILocation(line: 57, column: 33, scope: !2687)
!2725 = !{!"derived->extraData", !"char*"}
!2726 = !DILocation(line: 57, column: 14, scope: !2687)
!2727 = !DILocation(line: 57, column: 24, scope: !2687)
!2728 = !{!2729, !2069, i64 16}
!2729 = !{!"_ZTS7Derived", !2709, i64 0, !2069, i64 16}
!2730 = !DILocation(line: 0, scope: !2057, inlinedAt: !2731)
!2731 = distinct !DILocation(line: 58, column: 5, scope: !2687)
!2732 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2731)
!2733 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2731)
!2734 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2731)
!2735 = !DILocation(line: 0, scope: !2057, inlinedAt: !2736)
!2736 = distinct !DILocation(line: 58, column: 21, scope: !2687)
!2737 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2736)
!2738 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2736)
!2739 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2736)
!2740 = !DILocation(line: 0, scope: !2057, inlinedAt: !2741)
!2741 = distinct !DILocation(line: 59, column: 5, scope: !2687)
!2742 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2741)
!2743 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2741)
!2744 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2741)
!2745 = !DILocation(line: 0, scope: !2057, inlinedAt: !2746)
!2746 = distinct !DILocation(line: 59, column: 24, scope: !2687)
!2747 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2746)
!2748 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2746)
!2749 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2746)
!2750 = !DILocation(line: 60, column: 1, scope: !2687)
!2751 = distinct !DISubprogram(name: "test_move_store", linkageName: "_Z15test_move_storev", scope: !2, file: !2, line: 71, type: !1447, scopeLine: 71, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2752)
!2752 = !{!2753, !2754}
!2753 = !DILocalVariable(name: "h1", scope: !2751, file: !2, line: 72, type: !988)
!2754 = !DILocalVariable(name: "h2", scope: !2751, file: !2, line: 73, type: !988)
!2755 = !DILocation(line: 72, column: 28, scope: !2751)
!2756 = !DILocation(line: 0, scope: !2751)
!2757 = !DILocation(line: 0, scope: !2057, inlinedAt: !2758)
!2758 = distinct !DILocation(line: 74, column: 5, scope: !2751)
!2759 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2758)
!2760 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2758)
!2761 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2758)
!2762 = !DILocalVariable(name: "this", arg: 1, scope: !2763, type: !2765, flags: DIFlagArtificial | DIFlagObjectPointer)
!2763 = distinct !DISubprogram(name: "~StringHolder", linkageName: "_ZN12StringHolderD2Ev", scope: !988, file: !2, line: 68, type: !1000, scopeLine: 68, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !999, retainedNodes: !2764)
!2764 = !{!2762}
!2765 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !988, size: 64)
!2766 = !DILocation(line: 0, scope: !2763, inlinedAt: !2767)
!2767 = distinct !DILocation(line: 75, column: 1, scope: !2751)
!2768 = !DILocation(line: 68, column: 23, scope: !2769, inlinedAt: !2767)
!2769 = distinct !DILexicalBlock(scope: !2763, file: !2, line: 68, column: 21)
!2770 = !DILocation(line: 0, scope: !2763, inlinedAt: !2771)
!2771 = distinct !DILocation(line: 75, column: 1, scope: !2751)
!2772 = !DILocation(line: 75, column: 1, scope: !2751)
!2773 = distinct !DISubprogram(name: "test_multi_inherit_store", linkageName: "_Z24test_multi_inherit_storev", scope: !2, file: !2, line: 82, type: !1447, scopeLine: 82, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !2774)
!2774 = !{!2775}
!2775 = !DILocalVariable(name: "obj", scope: !2773, file: !2, line: 83, type: !2776)
!2776 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2777, size: 64)
!2777 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "MultiDerived", file: !2, line: 80, size: 192, flags: DIFlagTypePassByValue, elements: !2778, identifier: "_ZTS12MultiDerived")
!2778 = !{!2779, !2783, !2787}
!2779 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !2777, baseType: !2780, flags: DIFlagPublic, extraData: i32 0)
!2780 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Base1", file: !2, line: 78, size: 64, flags: DIFlagTypePassByValue, elements: !2781, identifier: "_ZTS5Base1")
!2781 = !{!2782}
!2782 = !DIDerivedType(tag: DW_TAG_member, name: "data1", scope: !2780, file: !2, line: 78, baseType: !30, size: 64, flags: DIFlagPublic)
!2783 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !2777, baseType: !2784, offset: 64, flags: DIFlagPublic, extraData: i32 0)
!2784 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Base2", file: !2, line: 79, size: 64, flags: DIFlagTypePassByValue, elements: !2785, identifier: "_ZTS5Base2")
!2785 = !{!2786}
!2786 = !DIDerivedType(tag: DW_TAG_member, name: "data2", scope: !2784, file: !2, line: 79, baseType: !30, size: 64, flags: DIFlagPublic)
!2787 = !DIDerivedType(tag: DW_TAG_member, name: "data3", scope: !2777, file: !2, line: 80, baseType: !30, size: 64, offset: 128, flags: DIFlagPublic)
!2788 = !DILocation(line: 83, column: 25, scope: !2773)
!2789 = !{!"obj", !"MultiDerived*"}
!2790 = !DILocation(line: 0, scope: !2773)
!2791 = !DILocation(line: 84, column: 25, scope: !2773)
!2792 = !{!"obj->", !"Base1"}
!2793 = !DILocation(line: 84, column: 16, scope: !2773)
!2794 = !{!2795, !2069, i64 0}
!2795 = !{!"_ZTS5Base1", !2069, i64 0}
!2796 = !DILocation(line: 85, column: 25, scope: !2773)
!2797 = !{!"obj.", !"Base1"}
!2798 = !DILocation(line: 85, column: 10, scope: !2773)
!2799 = !DILocation(line: 85, column: 16, scope: !2773)
!2800 = !{!2801, !2069, i64 0}
!2801 = !{!"_ZTS5Base2", !2069, i64 0}
!2802 = !DILocation(line: 86, column: 25, scope: !2773)
!2803 = !{!"obj->data3", !"char*"}
!2804 = !DILocation(line: 86, column: 10, scope: !2773)
!2805 = !DILocation(line: 86, column: 16, scope: !2773)
!2806 = !{!2807, !2069, i64 16}
!2807 = !{!"_ZTS12MultiDerived", !2795, i64 0, !2801, i64 8, !2069, i64 16}
!2808 = !DILocation(line: 0, scope: !2057, inlinedAt: !2809)
!2809 = distinct !DILocation(line: 87, column: 5, scope: !2773)
!2810 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2809)
!2811 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2809)
!2812 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2809)
!2813 = !DILocation(line: 0, scope: !2057, inlinedAt: !2814)
!2814 = distinct !DILocation(line: 87, column: 20, scope: !2773)
!2815 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2814)
!2816 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2814)
!2817 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2814)
!2818 = !DILocation(line: 0, scope: !2057, inlinedAt: !2819)
!2819 = distinct !DILocation(line: 87, column: 42, scope: !2773)
!2820 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2819)
!2821 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2819)
!2822 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2819)
!2823 = !DILocation(line: 0, scope: !2057, inlinedAt: !2824)
!2824 = distinct !DILocation(line: 87, column: 64, scope: !2773)
!2825 = !DILocation(line: 16, column: 50, scope: !2057, inlinedAt: !2824)
!2826 = !DILocation(line: 16, column: 33, scope: !2057, inlinedAt: !2824)
!2827 = !DILocation(line: 16, column: 54, scope: !2057, inlinedAt: !2824)
!2828 = !DILocation(line: 88, column: 1, scope: !2773)
!2829 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 90, type: !977, scopeLine: 90, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, retainedNodes: !125)
!2830 = !DILocation(line: 91, column: 5, scope: !2829)
!2831 = !DILocation(line: 92, column: 5, scope: !2829)
!2832 = !DILocation(line: 93, column: 5, scope: !2829)
!2833 = !DILocation(line: 94, column: 5, scope: !2829)
!2834 = !DILocation(line: 95, column: 5, scope: !2829)
!2835 = !DILocation(line: 96, column: 5, scope: !2829)
!2836 = !DILocation(line: 97, column: 5, scope: !2829)
!2837 = !DILocation(line: 98, column: 5, scope: !2829)
!2838 = distinct !DISubprogram(name: "~Base", linkageName: "_ZN4BaseD0Ev", scope: !972, file: !2, line: 50, type: !981, scopeLine: 50, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !980, retainedNodes: !2839)
!2839 = !{!2840}
!2840 = !DILocalVariable(name: "this", arg: 1, scope: !2838, type: !2690, flags: DIFlagArtificial | DIFlagObjectPointer)
!2841 = !DILocation(line: 0, scope: !2838)
!2842 = !DILocation(line: 50, column: 46, scope: !2838)
!2843 = distinct !DISubprogram(name: "~Base", linkageName: "_ZN4BaseD2Ev", scope: !972, file: !2, line: 50, type: !981, scopeLine: 50, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !980, retainedNodes: !2844)
!2844 = !{!2845}
!2845 = !DILocalVariable(name: "this", arg: 1, scope: !2843, type: !2690, flags: DIFlagArtificial | DIFlagObjectPointer)
!2846 = !DILocation(line: 0, scope: !2843)
!2847 = !DILocation(line: 50, column: 46, scope: !2843)
!2848 = distinct !DISubprogram(name: "~Derived", linkageName: "_ZN7DerivedD0Ev", scope: !984, file: !2, line: 51, type: !2715, scopeLine: 51, flags: DIFlagArtificial | DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2849, retainedNodes: !2850)
!2849 = !DISubprogram(name: "~Derived", scope: !984, type: !2715, containingType: !984, virtualIndex: 0, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagVirtual | DISPFlagOptimized)
!2850 = !{!2851}
!2851 = !DILocalVariable(name: "this", arg: 1, scope: !2848, type: !2692, flags: DIFlagArtificial | DIFlagObjectPointer)
!2852 = !DILocation(line: 0, scope: !2848)
!2853 = !DILocation(line: 51, column: 7, scope: !2848)
!2854 = distinct !DISubprogram(name: "~__shared_ptr_pointer", linkageName: "_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEED0Ev", scope: !1237, file: !914, line: 230, type: !1290, scopeLine: 230, flags: DIFlagArtificial | DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2855, retainedNodes: !2856)
!2855 = !DISubprogram(name: "~__shared_ptr_pointer", scope: !1237, type: !1290, containingType: !1237, virtualIndex: 0, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagVirtual | DISPFlagOptimized)
!2856 = !{!2857}
!2857 = !DILocalVariable(name: "this", arg: 1, scope: !2854, type: !1314, flags: DIFlagArtificial | DIFlagObjectPointer)
!2858 = !DILocation(line: 0, scope: !2854)
!2859 = !DILocation(line: 230, column: 7, scope: !2854)
!2860 = distinct !DISubprogram(name: "__on_zero_shared", linkageName: "_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE16__on_zero_sharedEv", scope: !1237, file: !914, line: 261, type: !1290, scopeLine: 262, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !1289, retainedNodes: !2861)
!2861 = !{!2862}
!2862 = !DILocalVariable(name: "this", arg: 1, scope: !2860, type: !1314, flags: DIFlagArtificial | DIFlagObjectPointer)
!2863 = !DILocation(line: 0, scope: !2860)
!2864 = !DILocation(line: 263, column: 5, scope: !2860)
!2865 = !DILocalVariable(name: "this", arg: 1, scope: !2866, type: !848, flags: DIFlagArtificial | DIFlagObjectPointer)
!2866 = distinct !DISubprogram(name: "second", linkageName: "_ZNSt3__h17__compressed_pairIPcPFvPvEE6secondB6v15004Ev", scope: !825, file: !114, line: 130, type: !839, scopeLine: 130, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !838, retainedNodes: !2867)
!2867 = !{!2865}
!2868 = !DILocation(line: 0, scope: !2866, inlinedAt: !2869)
!2869 = distinct !DILocation(line: 263, column: 21, scope: !2860)
!2870 = !DILocation(line: 131, column: 33, scope: !2866, inlinedAt: !2869)
!2871 = !DILocation(line: 263, column: 21, scope: !2860)
!2872 = !DILocation(line: 263, column: 46, scope: !2860)
!2873 = !DILocation(line: 0, scope: !2866, inlinedAt: !2874)
!2874 = distinct !DILocation(line: 264, column: 21, scope: !2860)
!2875 = !DILocation(line: 265, column: 1, scope: !2860)
!2876 = distinct !DISubprogram(name: "__get_deleter", linkageName: "_ZNKSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE13__get_deleterERKSt9type_info", scope: !1237, file: !914, line: 252, type: !1281, scopeLine: 253, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !1280, retainedNodes: !2877)
!2877 = !{!2878, !2879}
!2878 = !DILocalVariable(name: "this", arg: 1, scope: !2876, type: !1330, flags: DIFlagArtificial | DIFlagObjectPointer)
!2879 = !DILocalVariable(name: "__t", arg: 2, scope: !2876, file: !914, line: 240, type: !1285)
!2880 = !DILocation(line: 0, scope: !2876)
!2881 = !DILocalVariable(name: "this", arg: 1, scope: !2882, type: !2889, flags: DIFlagArtificial | DIFlagObjectPointer)
!2882 = distinct !DISubprogram(name: "operator==", linkageName: "_ZNKSt9type_infoeqB6v15004ERKS_", scope: !1287, file: !1288, line: 335, type: !2883, scopeLine: 336, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2886, retainedNodes: !2887)
!2883 = !DISubroutineType(types: !2884)
!2884 = !{!58, !2885, !1285}
!2885 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1286, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!2886 = !DISubprogram(name: "operator==", linkageName: "_ZNKSt9type_infoeqB6v15004ERKS_", scope: !1287, file: !1288, line: 335, type: !2883, scopeLine: 335, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagOptimized)
!2887 = !{!2881, !2888}
!2888 = !DILocalVariable(name: "__arg", arg: 2, scope: !2882, file: !1288, line: 335, type: !1285)
!2889 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1286, size: 64)
!2890 = !DILocation(line: 0, scope: !2882, inlinedAt: !2891)
!2891 = distinct !DILocation(line: 254, column: 16, scope: !2876)
!2892 = !DILocation(line: 337, column: 27, scope: !2882, inlinedAt: !2891)
!2893 = !{!2894, !2069, i64 8}
!2894 = !{!"_ZTSSt9type_info", !2069, i64 8}
!2895 = !DILocalVariable(name: "__lhs", arg: 1, scope: !2896, file: !1288, line: 210, type: !2906)
!2896 = distinct !DISubprogram(name: "__eq", linkageName: "_ZNSt27__type_info_implementations13__unique_impl4__eqB6v15004EPKcS2_", scope: !2897, file: !1288, line: 210, type: !2914, scopeLine: 210, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !2913, retainedNodes: !2917)
!2897 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__unique_impl", scope: !2898, file: !1288, line: 204, size: 8, flags: DIFlagTypePassByValue, elements: !2899, identifier: "_ZTSNSt27__type_info_implementations13__unique_implE")
!2898 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__type_info_implementations", scope: !16, file: !1288, line: 191, size: 8, flags: DIFlagTypePassByValue, elements: !125, identifier: "_ZTSSt27__type_info_implementations")
!2899 = !{!2900, !2910, !2913, !2916}
!2900 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !2897, baseType: !2901, extraData: i32 0)
!2901 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__string_impl_base", scope: !2898, file: !1288, line: 192, size: 8, flags: DIFlagTypePassByValue, elements: !2902, identifier: "_ZTSNSt27__type_info_implementations18__string_impl_baseE")
!2902 = !{!2903, !2907}
!2903 = !DISubprogram(name: "__type_name_to_string", linkageName: "_ZNSt27__type_info_implementations18__string_impl_base21__type_name_to_stringB6v15004EPKc", scope: !2901, file: !1288, line: 195, type: !2904, scopeLine: 195, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2904 = !DISubroutineType(types: !2905)
!2905 = !{!1175, !2906}
!2906 = !DIDerivedType(tag: DW_TAG_typedef, name: "__type_name_t", scope: !2901, file: !1288, line: 193, baseType: !1175)
!2907 = !DISubprogram(name: "__string_to_type_name", linkageName: "_ZNSt27__type_info_implementations18__string_impl_base21__string_to_type_nameB6v15004EPKc", scope: !2901, file: !1288, line: 199, type: !2908, scopeLine: 199, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2908 = !DISubroutineType(types: !2909)
!2909 = !{!2906, !1175}
!2910 = !DISubprogram(name: "__hash", linkageName: "_ZNSt27__type_info_implementations13__unique_impl6__hashB6v15004EPKc", scope: !2897, file: !1288, line: 206, type: !2911, scopeLine: 206, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2911 = !DISubroutineType(types: !2912)
!2912 = !{!68, !2906}
!2913 = !DISubprogram(name: "__eq", linkageName: "_ZNSt27__type_info_implementations13__unique_impl4__eqB6v15004EPKcS2_", scope: !2897, file: !1288, line: 210, type: !2914, scopeLine: 210, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2914 = !DISubroutineType(types: !2915)
!2915 = !{!58, !2906, !2906}
!2916 = !DISubprogram(name: "__lt", linkageName: "_ZNSt27__type_info_implementations13__unique_impl4__ltB6v15004EPKcS2_", scope: !2897, file: !1288, line: 214, type: !2914, scopeLine: 214, flags: DIFlagPrototyped | DIFlagStaticMember, spFlags: DISPFlagOptimized)
!2917 = !{!2895, !2918}
!2918 = !DILocalVariable(name: "__rhs", arg: 2, scope: !2896, file: !1288, line: 210, type: !2906)
!2919 = !DILocation(line: 0, scope: !2896, inlinedAt: !2920)
!2920 = distinct !DILocation(line: 337, column: 14, scope: !2882, inlinedAt: !2891)
!2921 = !DILocation(line: 211, column: 20, scope: !2896, inlinedAt: !2920)
!2922 = !DILocation(line: 254, column: 12, scope: !2876)
!2923 = !DILocation(line: 254, column: 5, scope: !2876)
!2924 = distinct !DISubprogram(name: "__on_zero_shared_weak", linkageName: "_ZNSt3__h20__shared_ptr_pointerIPcPFvPvENS_9allocatorIcEEE21__on_zero_shared_weakEv", scope: !1237, file: !914, line: 269, type: !1290, scopeLine: 270, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !1292, retainedNodes: !2925)
!2925 = !{!2926, !2927}
!2926 = !DILocalVariable(name: "this", arg: 1, scope: !2924, type: !1314, flags: DIFlagArtificial | DIFlagObjectPointer)
!2927 = !DILocalVariable(name: "__a", scope: !2924, file: !914, line: 275, type: !2928)
!2928 = !DIDerivedType(tag: DW_TAG_typedef, name: "_Al", scope: !2924, file: !914, line: 271, baseType: !2929)
!2929 = !DIDerivedType(tag: DW_TAG_typedef, name: "other", scope: !2930, file: !47, line: 141, baseType: !1304)
!2930 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rebind<std::__h::__shared_ptr_pointer<char *, void (*)(void *), std::__h::allocator<char> > >", scope: !1151, file: !47, line: 140, size: 8, flags: DIFlagTypePassByValue, elements: !125, templateParams: !2931, identifier: "_ZTSNSt3__h9allocatorIcE6rebindINS_20__shared_ptr_pointerIPcPFvPvES1_EEEE")
!2931 = !{!2932}
!2932 = !DITemplateTypeParameter(name: "_Up", type: !1237)
!2933 = !DILocation(line: 275, column: 9, scope: !2924)
!2934 = !DILocation(line: 0, scope: !2924)
!2935 = !DILocalVariable(name: "this", arg: 1, scope: !2936, type: !2940, flags: DIFlagArtificial | DIFlagObjectPointer)
!2936 = distinct !DISubprogram(name: "deallocate", linkageName: "_ZNSt3__h9allocatorINS_20__shared_ptr_pointerIPcPFvPvENS0_IcEEEEE10deallocateB6v15004EPS7_m", scope: !1304, file: !47, line: 124, type: !1316, scopeLine: 124, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !10, declaration: !1315, retainedNodes: !2937)
!2937 = !{!2935, !2938, !2939}
!2938 = !DILocalVariable(name: "__p", arg: 2, scope: !2936, file: !47, line: 124, type: !1314)
!2939 = !DILocalVariable(name: "__n", arg: 3, scope: !2936, file: !47, line: 124, type: !68)
!2940 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1304, size: 64)
!2941 = !DILocation(line: 0, scope: !2936, inlinedAt: !2942)
!2942 = distinct !DILocation(line: 277, column: 9, scope: !2924)
!2943 = !DILocation(line: 0, scope: !2400, inlinedAt: !2944)
!2944 = distinct !DILocation(line: 128, column: 13, scope: !2945, inlinedAt: !2942)
!2945 = distinct !DILexicalBlock(scope: !2946, file: !47, line: 127, column: 16)
!2946 = distinct !DILexicalBlock(scope: !2936, file: !47, line: 125, column: 13)
!2947 = !DILocation(line: 0, scope: !2411, inlinedAt: !2948)
!2948 = distinct !DILocation(line: 290, column: 12, scope: !2400, inlinedAt: !2944)
!2949 = !DILocation(line: 0, scope: !2421, inlinedAt: !2950)
!2950 = distinct !DILocation(line: 280, column: 10, scope: !2411, inlinedAt: !2948)
!2951 = !DILocation(line: 256, column: 3, scope: !2421, inlinedAt: !2950)
!2952 = !DILocation(line: 278, column: 1, scope: !2924)