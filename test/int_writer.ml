open Core.Std

let buf = Bigstring.create 32

type 'a t =
  { test    : Test.any
  ; channel : Out_channel.t
  }

let out_channel type_name =
  Out_channel.create @@ Test.bin_file type_name

let write_int i writer =
  let open Test in
  let (T t) = writer.test in
  let len = t.writer buf ~pos:0 (t.of_int64 i) in
  let s = Bigstring.To_string.sub buf ~pos:0 ~len in
  Out_channel.output_string writer.channel s

let () =
  Unix.mkdir_p "_build/bin";
  let writers =
    List.fold_left Test.integers ~init:String.Map.empty ~f:
      (fun m test ->
        let open Test in
        let T t = test in
        let channel = out_channel t.name in
        Map.add m t.name { test; channel }) in
  Test.iter
    (fun type_name i64 ->
      match Map.find writers type_name with
      | Some writer -> write_int i64 writer
      | None -> ())
