func.func @compute_max_typer(%colA: memref<?xi32>, %colB: memref<?xi32>, %N: index) -> i32 {
  %int_min = arith.constant -2147483648 : i32
  %max_val = memref.alloca() : memref<i32>
  memref.store %int_min, %max_val[] : memref<i32>

  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index

  omp.parallel {
    %local_max = memref.alloca() : memref<i32>
    memref.store %int_min, %local_max[] : memref<i32>

    omp.wsloop bounds(%c0) to (%N) step (%c1) {
      ^bb0(%i: index):
        %a_val = memref.load %colA[%i] : memref<?xi32>
        %b_val = memref.load %colB[%i] : memref<?xi32>

        %val = arith.addi %a_val, %b_val : i32

        %current_local_max = memref.load %local_max[] : memref<i32>

        %is_greater = arith.cmpi sgt, %val, %current_local_max : i32
        %new_local_max = arith.select %is_greater, %val, %current_local_max : i32
        memref.store %new_local_max, %local_max[] : memref<i32>
        
        omp.yield
    }

    omp.critical {
      %final_local_max = memref.load %local_max[] : memref<i32>
      %current_global_max = memref.load %max_val[] : memref<i32>
      %is_global_greater = arith.cmpi sgt, %final_local_max, %current_global_max : i32
      %new_global_max = arith.select %is_global_greater, %final_local_max, %current_global_max : i32
      memref.store %new_global_max, %max_val[] : memref<i32>
    }
    
    omp.terminator
  }

  %result = memref.load %max_val[] : memref<i32>
  return %result : i32
}