open Core.Std

type t =
  { test   : Test.any
  ; buffer : Bigstring.t
  }

let read_bin type_name =
  let bin_file = Test.bin_file type_name in
  let st = Unix.stat bin_file in
  let buf = Bigstring.create (Int64.to_int_exn st.Unix.st_size) in
  let fd = Unix.openfile bin_file ~mode:[Unix.O_RDONLY] in
  Bigstring.really_read fd buf;
  Unix.close fd;
  buf

let read_int expected reader ~pos_ref =
  let open Test in
  let (T t) = reader.test in
  let actual = t.reader reader.buffer ~pos_ref |> t.to_int64 in
  if actual <> expected then
    failwith (sprintf "%s: expected %Ld, got %Ld\n%!" t.name expected actual);
  !pos_ref

let () =
  let readers =
    List.fold_left Test.integers ~init:String.Map.empty ~f:
      (fun m test ->
        let open Test in
        let T t = test in
        let buffer = read_bin t.name in
        Map.add m t.name { test; buffer }) in
  let pos_ref = ref 0 in
  let cur_type = ref "_no_type_" in
  Test.iter
    (fun type_name i64 ->
      if type_name <> !cur_type then begin
        cur_type := type_name;
        pos_ref  := 0
      end;
      match Map.find readers type_name with
      | Some reader -> pos_ref := read_int i64 reader ~pos_ref
      | None -> ())
