; ModuleID = 'spec_lib_imply.c'
source_filename = "spec_lib_imply.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @bar(i32 noundef %0, i32 noundef %1) #0 !dbg !10 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  call void @llvm.dbg.declare(metadata ptr %3, metadata !15, metadata !DIExpression()), !dbg !16
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata ptr %5, metadata !19, metadata !DIExpression()), !dbg !20
  store i32 5, ptr %5, align 4, !dbg !20
  %6 = load i32, ptr %5, align 4, !dbg !21
  %7 = add nsw i32 %6, 2, !dbg !21
  store i32 %7, ptr %5, align 4, !dbg !21
  %8 = load i32, ptr %3, align 4, !dbg !22
  %9 = add nsw i32 %8, 5, !dbg !22
  store i32 %9, ptr %3, align 4, !dbg !22
  %10 = load i32, ptr %3, align 4, !dbg !23
  %11 = load i32, ptr %4, align 4, !dbg !24
  %12 = add nsw i32 %10, %11, !dbg !25
  %13 = load i32, ptr %5, align 4, !dbg !26
  %14 = add nsw i32 %12, %13, !dbg !27
  ret i32 %14, !dbg !28
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "Ubuntu clang version 15.0.7", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "spec_lib_imply.c", directory: "/mnt/c/Users/MensingRA/repos/pallas_spec2ir/examples", checksumkind: CSK_MD5, checksum: "3e6bf9cc116480d454d82a8454b30ca0")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 15.0.7"}
!10 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 9, type: !11, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !13, !13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !1, line: 9, type: !13)
!16 = !DILocation(line: 9, column: 14, scope: !10)
!17 = !DILocalVariable(name: "b", arg: 2, scope: !10, file: !1, line: 9, type: !13)
!18 = !DILocation(line: 9, column: 21, scope: !10)
!19 = !DILocalVariable(name: "c", scope: !10, file: !1, line: 10, type: !13)
!20 = !DILocation(line: 10, column: 9, scope: !10)
!21 = !DILocation(line: 11, column: 7, scope: !10)
!22 = !DILocation(line: 12, column: 7, scope: !10)
!23 = !DILocation(line: 13, column: 12, scope: !10)
!24 = !DILocation(line: 13, column: 16, scope: !10)
!25 = !DILocation(line: 13, column: 14, scope: !10)
!26 = !DILocation(line: 13, column: 20, scope: !10)
!27 = !DILocation(line: 13, column: 18, scope: !10)
!28 = !DILocation(line: 13, column: 5, scope: !10)
