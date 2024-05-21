; ModuleID = 'examples/spec_block.ll'
source_filename = "examples/spec_block.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo() #0 !dbg !10 {
  call void @llvm.dbg.value(metadata i32 7, metadata !15, metadata !DIExpression()), !dbg !16
  ret i32 7, !dbg !17
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @bar(i32 noundef %0, i32 noundef %1) #0 !dbg !18 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !21, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.value(metadata i32 %1, metadata !23, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.value(metadata i32 5, metadata !24, metadata !DIExpression()), !dbg !22
  %3 = add nsw i32 5, 2, !dbg !25
  call void @llvm.dbg.value(metadata i32 %3, metadata !24, metadata !DIExpression()), !dbg !22
  %4 = add nsw i32 %0, 5, !dbg !26
  call void @llvm.dbg.value(metadata i32 %4, metadata !21, metadata !DIExpression()), !dbg !22
  %5 = add nsw i32 %4, %1, !dbg !27
  %6 = add nsw i32 %5, %3, !dbg !28
  ret i32 %6, !dbg !29
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foobar(i32 noundef %0, i32 noundef %1) #0 !dbg !30 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !31, metadata !DIExpression()), !dbg !32
  call void @llvm.dbg.value(metadata i32 %1, metadata !33, metadata !DIExpression()), !dbg !32
  %3 = icmp sgt i32 %0, %1, !dbg !34
  br i1 %3, label %4, label %5, !dbg !36

4:                                                ; preds = %2
  br label %6, !dbg !37

5:                                                ; preds = %2
  br label %6, !dbg !39

6:                                                ; preds = %5, %4
  %.0 = phi i32 [ %0, %4 ], [ 3, %5 ], !dbg !41
  ret i32 %.0, !dbg !42
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "Ubuntu clang version 15.0.7", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "examples/spec_block.c", directory: "/mnt/c/Users/MensingRA/repos/pallas_spec2ir", checksumkind: CSK_MD5, checksum: "bacf9dccc75131003916938b27e1b383")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 15.0.7"}
!10 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 9, type: !11, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "v", scope: !10, file: !1, line: 10, type: !13)
!16 = !DILocation(line: 0, scope: !10)
!17 = !DILocation(line: 11, column: 5, scope: !10)
!18 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 18, type: !19, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!19 = !DISubroutineType(types: !20)
!20 = !{!13, !13, !13}
!21 = !DILocalVariable(name: "a", arg: 1, scope: !18, file: !1, line: 18, type: !13)
!22 = !DILocation(line: 0, scope: !18)
!23 = !DILocalVariable(name: "b", arg: 2, scope: !18, file: !1, line: 18, type: !13)
!24 = !DILocalVariable(name: "c", scope: !18, file: !1, line: 19, type: !13)
!25 = !DILocation(line: 20, column: 7, scope: !18)
!26 = !DILocation(line: 21, column: 7, scope: !18)
!27 = !DILocation(line: 22, column: 14, scope: !18)
!28 = !DILocation(line: 22, column: 18, scope: !18)
!29 = !DILocation(line: 22, column: 5, scope: !18)
!30 = distinct !DISubprogram(name: "foobar", scope: !1, file: !1, line: 26, type: !19, scopeLine: 26, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!31 = !DILocalVariable(name: "a", arg: 1, scope: !30, file: !1, line: 26, type: !13)
!32 = !DILocation(line: 0, scope: !30)
!33 = !DILocalVariable(name: "b", arg: 2, scope: !30, file: !1, line: 26, type: !13)
!34 = !DILocation(line: 27, column: 11, scope: !35)
!35 = distinct !DILexicalBlock(scope: !30, file: !1, line: 27, column: 9)
!36 = !DILocation(line: 27, column: 9, scope: !30)
!37 = !DILocation(line: 28, column: 9, scope: !38)
!38 = distinct !DILexicalBlock(scope: !35, file: !1, line: 27, column: 16)
!39 = !DILocation(line: 30, column: 9, scope: !40)
!40 = distinct !DILexicalBlock(scope: !35, file: !1, line: 29, column: 12)
!41 = !DILocation(line: 0, scope: !35)
!42 = !DILocation(line: 32, column: 1, scope: !30)
