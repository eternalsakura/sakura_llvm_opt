; ModuleID = 'available-test-m2r.ll'
source_filename = "available-test-m2r.bc"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

; Function Attrs: nounwind
define i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %add = add nsw i32 %argc, 50
  %add1 = add nsw i32 %add, 96
  %cmp = icmp slt i32 50, %add
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %sub = sub nsw i32 %add, 50
  %mul = mul nsw i32 96, %add
  br label %if.end

if.else:                                          ; preds = %entry
  %add2 = add nsw i32 %add, 50
  %mul3 = mul nsw i32 96, %add
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %f.0 = phi i32 [ %sub, %if.then ], [ %add2, %if.else ]
  %sub4 = sub nsw i32 50, 96
  %add5 = add nsw i32 %sub4, %f.0
  ret i32 0
}

attributes #0 = { nounwind "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.5.0 (branches/release_35 225468)"}
