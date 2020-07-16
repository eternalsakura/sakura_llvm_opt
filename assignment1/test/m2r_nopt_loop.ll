; ModuleID = 'nopt_loop.ll'
source_filename = "loop.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

@g = common global i32 0, align 4

; Function Attrs: noinline nounwind ssp uwtable
define i32 @g_incr(i32 %0) #0 {
  %2 = load i32, i32* @g, align 4
  %3 = add nsw i32 %2, %0
  store i32 %3, i32* @g, align 4
  %4 = load i32, i32* @g, align 4
  ret i32 %4
}

; Function Attrs: noinline nounwind ssp uwtable
define i32 @loop(i32 %0, i32 %1, i32 %2) #0 {
  br label %4

4:                                                ; preds = %8, %3
  %.0 = phi i32 [ %0, %3 ], [ %9, %8 ]
  %5 = icmp slt i32 %.0, %1
  br i1 %5, label %6, label %10

6:                                                ; preds = %4
  %7 = call i32 @g_incr(i32 %2)
  br label %8

8:                                                ; preds = %6
  %9 = add nsw i32 %.0, 1
  br label %4

10:                                               ; preds = %4
  %11 = load i32, i32* @g, align 4
  %12 = add nsw i32 0, %11
  ret i32 %12
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 10.0.0 "}
