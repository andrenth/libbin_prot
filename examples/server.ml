open Core
open Async

module Hello = struct
  let rpc = Rpc.Rpc.create
    ~name:"hello-world"
    ~version:0
    ~bin_query:String.bin_t
    ~bin_response:String.bin_t

  let rpc_impl () s =
    return (s ^ ", world")
end

module Incr = struct
  let rpc = Rpc.Rpc.create
    ~name:"incr"
    ~version:0
    ~bin_query:Int.bin_t
    ~bin_response:Int.bin_t

  let rpc_impl () i =
    return (i + 1)
end

let implementations =
  [ Rpc.Rpc.implement Hello.rpc Hello.rpc_impl
  ; Rpc.Rpc.implement Incr.rpc  Incr.rpc_impl
  ]

let port_arg () =
  Command.Spec.(
    flag "-port" (optional_with_default 8124 int)
      ~doc:" Server's port"
  )

let start_server ~env ?(stop=Deferred.never ()) ~implementations ~port () =
  Log.Global.info "Starting server on %d" port;
  let implementations =
    Rpc.Implementations.create_exn ~implementations
      ~on_unknown_rpc:(`Call (fun _st ~rpc_tag ~version ->
        Log.Global.info "Unexpected RPC, tag %s, version %d" rpc_tag version;
        `Continue))
  in
  Tcp.Server.create
    ~on_handler_error:(`Call (fun _ exn -> Log.Global.sexp (Exn.sexp_of_t exn)))
    (Tcp.on_port port)
    (fun _addr r w ->
      Rpc.Connection.server_with_close r w
        ~connection_state:(fun _ -> env)
        ~on_handshake_error:(
          `Call (fun exn -> Log.Global.sexp (Exn.sexp_of_t exn); return ()))
        ~implementations
    )
  >>= fun server ->
  Log.Global.info "Server started, waiting for close";
  Deferred.any
    [ (stop >>= fun () -> Tcp.Server.close server)
    ; Tcp.Server.close_finished server ]

let command =
  Command.async
    ~summary:"Example server"
    Command.Spec.(
      empty +> port_arg ()
    )
    (fun port () -> start_server ~env:() ~port ~implementations ())

let () = Command.run command
