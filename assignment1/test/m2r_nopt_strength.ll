; ModuleID = 'nopt_strength.ll'
source_filename = "strength.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

; Function Attrs: noinline nounwind ssp uwtable
define i32 @compute(i32 %0, i32 %1) #0 {
  %3 = mul nsw i32 %0, 2
  %4 = add nsw i32 0, %3
  %5 = mul nsw i32 %0, 3
  %6 = add nsw i32 %4, %5
  %7 = mul nsw i32 %0, 8
  %8 = add nsw i32 %6, %7
  %9 = sdiv i32 %1, 2
  %10 = sub nsw i32 %8, %9
  %11 = sdiv i32 %1, 4
  %12 = sub nsw i32 %10, %11
  %13 = sdiv i32 %1, 8
  %14 = sub nsw i32 %12, %13
  ret i32 %14
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 10.0.0 "}
