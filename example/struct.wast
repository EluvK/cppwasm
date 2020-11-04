(module
 (type $FUNCSIG$vi (func (param i32)))
 (import "env" "print" (func $print (param i32)))
 (table 0 anyfunc)
 (memory $0 1)
 (export "memory" (memory $0))
 (export "test" (func $test))
 (func $test (; 1 ;) (param $0 i32) (param $1 i32)
  (local $2 i64)
  (local $3 i32)
  (i32.store offset=4
   (i32.const 0)
   (tee_local $3
    (i32.sub
     (i32.load offset=4
      (i32.const 0)
     )
     (i32.const 16)
    )
   )
  )
  (i64.store offset=8
   (get_local $3)
   (tee_local $2
    (i64.or
     (i64.shl
      (i64.extend_u/i32
       (get_local $1)
      )
      (i64.const 32)
     )
     (i64.extend_u/i32
      (get_local $0)
     )
    )
   )
  )
  (i64.store align=4
   (get_local $3)
   (get_local $2)
  )
  (call $print
   (get_local $3)
  )
  (i32.store offset=4
   (i32.const 0)
   (i32.add
    (get_local $3)
    (i32.const 16)
   )
  )
 )
)
