open Core.Std

module Read  = Bin_prot.Read
module Write = Bin_prot.Write
module Nat0  = Bin_prot.Nat0

type 'a t =
  { name     : string
  ; reader   : 'a Read.reader
  ; writer   : 'a Write.writer
  ; to_int64 : 'a -> Int64.t
  ; of_int64 : Int64.t -> 'a
  }

type any = T : _ t -> any

let expectations = "test/integers_repr.expected"

let bin_file type_name =
  sprintf "_build/bin/%s.bin" type_name

let integers =
  [ T { name     = "int"
      ; reader   = Read.bin_read_int
      ; writer   = Write.bin_write_int
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "int32"
      ; reader   = Read.bin_read_int32
      ; writer   = Write.bin_write_int32
      ; to_int64 = Int64.of_int32
      ; of_int64 = Int64.to_int32_exn
      }
  ; T { name     = "int64"
      ; reader   = Read.bin_read_int64
      ; writer   = Write.bin_write_int64
      ; to_int64 = Fn.id
      ; of_int64 = Fn.id
      }
  ; T { name     = "int_16bit"
      ; reader   = Read.bin_read_int_16bit
      ; writer   = Write.bin_write_int_16bit
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "int_32bit"
      ; reader   = Read.bin_read_int_32bit
      ; writer   = Write.bin_write_int_32bit
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "int_64bit"
      ; reader   = Read.bin_read_int_64bit
      ; writer   = Write.bin_write_int_64bit
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "int64_bits"
      ; reader   = Read.bin_read_int64_bits
      ; writer   = Write.bin_write_int64_bits
      ; to_int64 = Fn.id
      ; of_int64 = Fn.id
      }
  ; T { name     = "network16_int"
      ; reader   = Read.bin_read_network16_int
      ; writer   = Write.bin_write_network16_int
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "network32_int"
      ; reader   = Read.bin_read_network32_int
      ; writer   = Write.bin_write_network32_int
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "network32_int32"
      ; reader   = Read.bin_read_network32_int32
      ; writer   = Write.bin_write_network32_int32
      ; to_int64 = Int64.of_int32
      ; of_int64 = Int64.to_int32_exn
      }
  ; T { name     = "network64_int"
      ; reader   = Read.bin_read_network64_int
      ; writer   = Write.bin_write_network64_int
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ; T { name     = "network64_int64"
      ; reader   = Read.bin_read_network64_int64
      ; writer   = Write.bin_write_network64_int64
      ; to_int64 = Fn.id
      ; of_int64 = Fn.id
      }
  ; T { name     = "nat0"
      ; reader   = Read.bin_read_nat0
      ; writer   = Write.bin_write_nat0
      ; to_int64 = (fun x -> Int64.of_int (x : Nat0.t :> int))
      ; of_int64 = (fun x -> Nat0.of_int (Int64.to_int_exn x))
      }
  ; T { name     = "variant_int"
      ; reader   = Read.bin_read_variant_int
      ; writer   = Write.bin_write_variant_int
      ; to_int64 = Int64.of_int
      ; of_int64 = Int64.to_int_exn
      }
  ]

let iter f =
  let ic = In_channel.create expectations in
  In_channel.iter_lines ic ~f:
    (fun line ->
      match String.split line ~on:'|' with
      | [type_name; rest] ->
          let i =
            String.split rest ~on:' '
            |> List.last_exn
            |> Int64.of_string in
          f type_name i
      | _ ->
          failwithf "bad input line '%s'" line ())
