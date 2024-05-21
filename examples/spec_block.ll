; ModuleID = 'examples/spec_block.c'
source_filename = "examples/spec_block.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo() #0 !dbg !10 {
  %1 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata ptr %1, metadata !15, metadata !DIExpression()), !dbg !16
  store i32 7, ptr %1, align 4, !dbg !16
  %2 = load i32, ptr %1, align 4, !dbg !17
  ret i32 %2, !dbg !18
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @bar(i32 noundef %0, i32 noundef %1) #0 !dbg !19 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  call void @llvm.dbg.declare(metadata ptr %3, metadata !22, metadata !DIExpression()), !dbg !23
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.declare(metadata ptr %5, metadata !26, metadata !DIExpression()), !dbg !27
  store i32 5, ptr %5, align 4, !dbg !27
  %6 = load i32, ptr %5, align 4, !dbg !28
  %7 = add nsw i32 %6, 2, !dbg !28
  store i32 %7, ptr %5, align 4, !dbg !28
  %8 = load i32, ptr %3, align 4, !dbg !29
  %9 = add nsw i32 %8, 5, !dbg !29
  store i32 %9, ptr %3, align 4, !dbg !29
  %10 = load i32, ptr %3, align 4, !dbg !30
  %11 = load i32, ptr %4, align 4, !dbg !31
  %12 = add nsw i32 %10, %11, !dbg !32
  %13 = load i32, ptr %5, align 4, !dbg !33
  %14 = add nsw i32 %12, %13, !dbg !34
  ret i32 %14, !dbg !35
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foobar(i32 noundef %0, i32 noundef %1) #0 !dbg !36 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !37, metadata !DIExpression()), !dbg !38
  store i32 %1, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %5, metadata !39, metadata !DIExpression()), !dbg !40
  %6 = load i32, ptr %4, align 4, !dbg !41
  %7 = load i32, ptr %5, align 4, !dbg !43
  %8 = icmp sgt i32 %6, %7, !dbg !44
  br i1 %8, label %9, label %11, !dbg !45

9:                                                ; preds = %2
  %10 = load i32, ptr %4, align 4, !dbg !46
  store i32 %10, ptr %3, align 4, !dbg !48
  br label %12, !dbg !48

11:                                               ; preds = %2
  store i32 3, ptr %3, align 4, !dbg !49
  br label %12, !dbg !49

12:                                               ; preds = %11, %9
  %13 = load i32, ptr %3, align 4, !dbg !51
  ret i32 %13, !dbg !51
}

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
!16 = !DILocation(line: 10, column: 9, scope: !10)
!17 = !DILocation(line: 11, column: 12, scope: !10)
!18 = !DILocation(line: 11, column: 5, scope: !10)
!19 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 18, type: !20, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!20 = !DISubroutineType(types: !21)
!21 = !{!13, !13, !13}
!22 = !DILocalVariable(name: "a", arg: 1, scope: !19, file: !1, line: 18, type: !13)
!23 = !DILocation(line: 18, column: 14, scope: !19)
!24 = !DILocalVariable(name: "b", arg: 2, scope: !19, file: !1, line: 18, type: !13)
!25 = !DILocation(line: 18, column: 21, scope: !19)
!26 = !DILocalVariable(name: "c", scope: !19, file: !1, line: 19, type: !13)
!27 = !DILocation(line: 19, column: 9, scope: !19)
!28 = !DILocation(line: 20, column: 7, scope: !19)
!29 = !DILocation(line: 21, column: 7, scope: !19)
!30 = !DILocation(line: 22, column: 12, scope: !19)
!31 = !DILocation(line: 22, column: 16, scope: !19)
!32 = !DILocation(line: 22, column: 14, scope: !19)
!33 = !DILocation(line: 22, column: 20, scope: !19)
!34 = !DILocation(line: 22, column: 18, scope: !19)
!35 = !DILocation(line: 22, column: 5, scope: !19)
!36 = distinct !DISubprogram(name: "foobar", scope: !1, file: !1, line: 26, type: !20, scopeLine: 26, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!37 = !DILocalVariable(name: "a", arg: 1, scope: !36, file: !1, line: 26, type: !13)
!38 = !DILocation(line: 26, column: 16, scope: !36)
!39 = !DILocalVariable(name: "b", arg: 2, scope: !36, file: !1, line: 26, type: !13)
!40 = !DILocation(line: 26, column: 23, scope: !36)
!41 = !DILocation(line: 27, column: 9, scope: !42)
!42 = distinct !DILexicalBlock(scope: !36, file: !1, line: 27, column: 9)
!43 = !DILocation(line: 27, column: 13, scope: !42)
!44 = !DILocation(line: 27, column: 11, scope: !42)
!45 = !DILocation(line: 27, column: 9, scope: !36)
!46 = !DILocation(line: 28, column: 16, scope: !47)
!47 = distinct !DILexicalBlock(scope: !42, file: !1, line: 27, column: 16)
!48 = !DILocation(line: 28, column: 9, scope: !47)
!49 = !DILocation(line: 30, column: 9, scope: !50)
!50 = distinct !DILexicalBlock(scope: !42, file: !1, line: 29, column: 12)
!51 = !DILocation(line: 32, column: 1, scope: !36)
