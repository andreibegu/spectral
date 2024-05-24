; ModuleID = 'tmp_driver.c'
source_filename = "tmp_driver.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local zeroext i1 @PALLAS_SPEC_1(i32 noundef %0, i32 noundef %1) #0 !dbg !10 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  call void @llvm.dbg.declare(metadata ptr %3, metadata !16, metadata !DIExpression()), !dbg !17
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !18, metadata !DIExpression()), !dbg !19
  %5 = load i32, ptr %3, align 4, !dbg !20
  %6 = icmp sge i32 0, %5, !dbg !21
  br i1 %6, label %7, label %11, !dbg !22

7:                                                ; preds = %2
  %8 = load i32, ptr %3, align 4, !dbg !23
  %9 = load i32, ptr %4, align 4, !dbg !24
  %10 = icmp sge i32 %8, %9, !dbg !25
  br label %11

11:                                               ; preds = %7, %2
  %12 = phi i1 [ false, %2 ], [ %10, %7 ], !dbg !26
  ret i1 %12, !dbg !27
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "Ubuntu clang version 15.0.7", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "tmp_driver.c", directory: "/mnt/c/Users/MensingRA/repos/pallas_spec2ir/examples", checksumkind: CSK_MD5, checksum: "ee2b3a642b6d94ca7adf3f691d9f3f39")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 15.0.7"}
!10 = distinct !DISubprogram(name: "PALLAS_SPEC_1", scope: !1, file: !1, line: 2, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !14, !14}
!13 = !DIBasicType(name: "_Bool", size: 8, encoding: DW_ATE_boolean)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !1, line: 2, type: !14)
!17 = !DILocation(line: 2, column: 25, scope: !10)
!18 = !DILocalVariable(name: "b", arg: 2, scope: !10, file: !1, line: 2, type: !14)
!19 = !DILocation(line: 2, column: 32, scope: !10)
!20 = !DILocation(line: 4, column: 14, scope: !10)
!21 = !DILocation(line: 4, column: 11, scope: !10)
!22 = !DILocation(line: 4, column: 16, scope: !10)
!23 = !DILocation(line: 4, column: 19, scope: !10)
!24 = !DILocation(line: 4, column: 24, scope: !10)
!25 = !DILocation(line: 4, column: 21, scope: !10)
!26 = !DILocation(line: 0, scope: !10)
!27 = !DILocation(line: 4, column: 2, scope: !10)
