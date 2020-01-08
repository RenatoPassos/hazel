open UHExp;

// TODO: Discard the "?" holes in the first step
//  Then carefully consider it

// TODO: Use auto-formatter in ocaml to deal with unnecessary indent and ()s

// Add indent levels and pass into the handlers
// level starts from 0, +1 means double space
let rec indent_space = (~level: int): string =>
  if (level > 0) {
    "  " ++ indent_space(~level=level - 1);
  } else {
    "";
  };

// FIXME:TODO: use it to simplify
// option_string_concat([s1, Some(constant)])
let rec option_string_concat =
        (~strs: list(option(string))): option(string) =>
  switch (strs) {
  | [] => Some("")
  | [a, ...rest] =>
    switch (a, option_string_concat(~strs=rest)) {
    | (Some(s1), Some(s2)) => Some(s1 ++ s2)
    | _ => None
    }
  };
//==============================
//  UHTyp.re
//==============================

// translate type constructor into ocaml type
//opseq is when using bi-argument operations like plus,
//skel.t use BinOp to indicate the operation(op) and two placeholder
//  in UHTyp, it's one the type, like Num -> Num
//  It's like skel.t will always receive NotInHole, so ignore it
//opseq is the operation, ExpOpExp is to do calculation,
//  SeqOpExp is to do sequence operation, like 1+2+3
let rec uhtyp_translater = (~t: UHTyp.t): option(string) =>
  switch (t) {
  | Hole => None
  | Num => Some("int")
  | Bool => Some("bool")
  | Unit => Some("()") //written as (), actually is unit
  | List(a) =>
    switch (uhtyp_translater(~t=a)) {
    | None => None
    | Some(s) => Some(s ++ " list")
    }
  | Parenthesized(a) =>
    switch (uhtyp_translater(~t=a)) {
    | None => None
    | Some(s) => Some("(" ++ s ++ ")")
    }
  | OpSeq(skel_t, opseq) =>
    switch (skel_t) {
    // Since skeleton is consistant with opseq, decline skel_t
    | BinOp(NotInHole, _, _, _) => uhtyp_opseq_translater(~opseq)
    | _ => None
    }
  }
and uhtyp_opseq_translater = (~opseq): option(string) =>
  switch (opseq) {
  | ExpOpExp(tm1, op, tm2) =>
    switch (uhtyp_translater(~t=tm1), uhtyp_translater(~t=tm2)) {
    | (Some(a), Some(b)) =>
      Some(a ++ " " ++ uhtyp_op_translater(~op) ++ " " ++ b)
    | _ => None
    }
  | SeqOpExp(seq, op, tm) =>
    switch (uhtyp_opseq_translater(~opseq=seq), uhtyp_translater(~t=tm)) {
    | (Some(a), Some(b)) =>
      Some(a ++ " " ++ uhtyp_op_translater(~op) ++ " " ++ b)
    | _ => None
    }
  }
and uhtyp_op_translater = (~op: UHTyp.op): string =>
  switch (op) {
  | Arrow => " -> "
  | Prod => " * " //int*int in ocaml
  | Sum => " | "
  };

//FIXME: Here is a version with type inference
//  such as (\lambda x:?.x+1) 1 can return 2, but leave a ? hole as "Hole"
//ocaml can do type inference, so just change hole to 'a
// let rec uhtyp_translater = (~t : UHTyp.t) : option(string) =>
//   switch(t) {
//     | Hole => "'a"
//     | Num => Some("int")
//     | Bool => Some("bool")
//     | Unit => Some("()") //written as (), actually is unit
//     | List(a) => switch(uhtyp_translater(a)){
//       | None => None
//       | Some(s) => Some(s ++ " list")
//     }
//     | Parenthesized(a) => switch(uhtyp_translater(a)) {
//       | None => None
//       | Some(s) => Some("(" ++ s ++ ")")
//     }
//     | OpSeq(skel_t, opseq) => switch(skel_t){
//       | BinOp(NotInHole, _, _, _) => uhtyp_opseq_translater(~opseq=opseq)
//       | _ => None
//     }
//   }
//   and uhtyp_opseq_translater = (~opseq) : option(string) => switch(opseq){
//         | ExpOpExp(tm1, op, tm2) => switch((uhtyp_translater(tm1), uhtyp_translater(tm2))){
//           | (Some(a), Some(b)) => Some(a ++ " " ++ uhtyp_op_translater(op) ++ " " ++ b)
//           | _ => None
//         }
//         | SeqOpExp(seq, op, tm) => switch(uhtyp_opseq_translater(~opseq=seq), uhtyp_translater(tm)) {
//           | (Some(a), Some(b)) => Some(a ++ " " ++ uhtyp_op_translater(op) ++ " " ++ b)
//           | _ => None
//         }
//   } and uhtyp_op_translater = (~op : UHTyp.op) : string =>
//   switch(op) {
//     | Arrow => " -> "
//     | Prod => " * " //int*int in ocaml
//     | Sum => " | "
//   };

//==============================
// UHPat.re
//==============================

let rec uhpat_translater = (~t: UHPat.t): option(string) =>
  switch (t) {
  | EmptyHole(_) => None
  | Wild(a) =>
    switch (a) {
    | NotInHole => Some("_")
    | _ => None
    }
  | Var(a, b, c) =>
    switch (a, b) {
    | (NotInHole, NotInVarHole) => Some(c)
    | _ => None
    }
  | NumLit(a, b) =>
    switch (a) {
    | NotInHole => Some(string_of_int(b))
    | _ => None
    }
  | BoolLit(a, b) =>
    switch (a) {
    | NotInHole => Some(string_of_bool(b))
    | _ => None
    }
  | ListNil(a) =>
    switch (a) {
    | NotInHole => Some("[]")
    | _ => None
    }
  | Parenthesized(t) =>
    switch (uhpat_translater(~t)) {
    | None => None
    | Some(s) => Some("(" ++ s ++ ")")
    }
  //FIXME: currently we use polymorphic type ('a) for it in "Let" assignment,
  // better to reconstruct for a type
  // (though the inference is good for that)
  // in Hazel, a type Num | Bool can't have value 1,
  //   it should be type inj[L or R, Bool](1)
  // in ocaml, sum type should be directly give value by constructor
  //   and need a type declaration for sum type
  // Hence change "x : (A | B) = inj[L](val)" where val:A into
  //   "x : 'a = val" temporarily, :'a can be ignored
  //   here we ignore 'a, because imagine assignment is legal
  | Inj(a, _b, c) =>
    switch (a) {
    | NotInHole => uhpat_translater(~t=c)
    | _ => None
    }
  | OpSeq(skel_t, opseq) =>
    switch (skel_t) {
    | BinOp(NotInHole, _, _, _) => uhpat_opseq_translater(~opseq)
    | _ => None
    }
  }
and uhpat_opseq_translater = (~opseq): option(string) =>
  switch (opseq) {
  | ExpOpExp(tm1, op, tm2) =>
    switch (uhpat_translater(~t=tm1), uhpat_translater(~t=tm2)) {
    | (Some(a), Some(b)) =>
      Some(a ++ " " ++ uhpat_op_translater(~op) ++ " " ++ b)
    | _ => None
    }
  | SeqOpExp(seq, op, tm) =>
    switch (uhpat_opseq_translater(~opseq=seq), uhpat_translater(~t=tm)) {
    | (Some(a), Some(b)) =>
      Some(a ++ " " ++ uhpat_op_translater(~op) ++ " " ++ b)
    | _ => None
    }
  }
and uhpat_op_translater = (~op: UHPat.op): string =>
  switch (op) {
  | Comma => ", "
  | Space => " "
  | Cons => " :: "
  };

//==============================
//  UHExp.re
//==============================

// using "fun list(_ : type) -> expr" as lambda expression
// nested is ok, fun x -> fun y -> ... -> expr

let uhexp_op_translater = (~op: UHExp.op): string =>
  switch (op) {
  | Space => " "
  | Plus => "+"
  | Minus => "-"
  | Times => "*"
  | LessThan => "<"
  | GreaterThan => ">"
  | Equals => "=="
  | Comma => ", "
  | Cons => "::"
  | And => "&&"
  | Or => "||"
  };

//TODO: complete the block and line part, add indent level indicater
//      We should force that Block([], outer nodes) should have indent 0
//      In each recursive call, we will proceed indent level by 1
//Logic: every block when it's called don't need to indent,
//        but the start of every line need to indent
//        lines follow a expression, which should be same level
//        and end of a lock doesn't insert \n
//        so indent after each line ends
let rec block_handler = (~block: block, ~level: int): option(string) =>
  switch (block) {
  | Block(lines, t) =>
    option_string_concat(
      ~strs=[lines_handler(~lines, ~level), type_handler(~t, ~level)],
    )
  }
//The t part
and type_handler = (~t: t, ~level: int): option(string) =>
  //Not all items need the level
  //If it's a expression and no lines, no indent will apply
  switch (t) {
  // outer nodes
  | EmptyHole(_) => None //an empty hole always means incomplete
  | Var(a, b, c) =>
    switch (a, b) {
    | (NotInHole, NotInVarHole) => Some(c)
    | _ => None
    }
  | NumLit(a, b) =>
    switch (a) {
    | NotInHole => Some(string_of_int(b))
    | _ => None
    }
  | BoolLit(a, b) =>
    switch (a) {
    | NotInHole => Some(string_of_bool(b))
    | _ => None
    }
  | ListNil(a) =>
    switch (a) {
    | NotInHole => Some("[]")
    | _ => None
    }
  // inner nodes
  | Lam(a, b, c, d) =>
    lam_handler(~errstatus=a, ~uhpat=b, ~uhtyp=c, ~block=d, ~level)
  | Inj(a, _b, c) => inj_handler(~errstatus=a, ~block=c, ~level)
  | Case(a, b, c, d) =>
    switch (a) {
    | NotInHole => case_handler(~block=b, ~rules=c, ~uhtyp=d, ~level)
    | _ => None
    }
  | Parenthesized(b) =>
    switch (block_handler(~block=b, ~level)) {
    | None => None
    | Some(s) => Some("(" ++ s ++ ")")
    }
  | OpSeq(skel_t, opseq) =>
    switch (skel_t) {
    //since invariant of skel_t and opseq, decline skel_t
    | BinOp(NotInHole, _, _, _) => opseq_handler(~opseq, ~level)
    | _ => None
    }
  //TODO: ApPalette (Discard it as exceptions, since it isn't implemented now)
  | _ => None
  }
// Lam helper function
and lam_handler =
    (
      ~errstatus: ErrStatus.t,
      ~uhpat: UHPat.t,
      ~uhtyp: option(UHTyp.t),
      ~block: block,
      ~level: int,
    )
    : option(string) =>
  //UHTyp we receive a Some(Hole), it maybe legal to inference
  //Just use another version of uhtyp_translator
  switch (errstatus, uhpat, uhtyp) {
  | (NotInHole, pat, None) =>
    switch (
      uhpat_translater(~t=pat),
      block_handler(~block, ~level=level + 1),
    ) {
    //here we don't need indent because it's follow some expression like "="
    //but maybe writting it in a new line needs
    //FIXME: currently insert an () to protect codes, figure out whether can remove
    | (Some(s), Some(b)) => Some("(fun " ++ s ++ " -> " ++ b ++ ")")
    | _ => None
    }
  | (NotInHole, pat, Some(typ)) =>
    switch (
      uhpat_translater(~t=pat),
      uhtyp_translater(~t=typ),
      block_handler(~block, ~level=level + 1),
    ) {
    //Maybe here need ()
    | (Some(s), Some(t), Some(b)) =>
      Some("(fun " ++ s ++ ":" ++ t ++ " -> " ++ b ++ ")")
    | _ => None
    }
  | _ => None
  }

and inj_handler =
    //Currently discard  ~injside: InjSide.t
    (~errstatus: ErrStatus.t, ~block: block, ~level: int): option(string) =>
  //FIXME: Another injection issue
  switch (errstatus) {
  | NotInHole => block_handler(~block, ~level=level + 1)
  | _ => None
  }

and opseq_handler = (~opseq: UHExp.opseq, ~level: int): option(string) =>
  switch (opseq) {
  | ExpOpExp(tm1, op, tm2) =>
    switch (
      type_handler(~t=tm1, ~level=level + 1),
      type_handler(~t=tm2, ~level=level + 1),
    ) {
    | (Some(a), Some(b)) =>
      Some(a ++ " " ++ uhexp_op_translater(~op) ++ " " ++ b)
    | _ => None
    }
  | SeqOpExp(seq, op, tm) =>
    switch (
      opseq_handler(~opseq=seq, ~level=level + 1),
      type_handler(~t=tm, ~level=level + 1),
    ) {
    | (Some(a), Some(b)) =>
      Some(a ++ " " ++ uhexp_op_translater(~op) ++ " " ++ b)
    | _ => None
    }
  }
//put errstatus check into the main function
and case_handler =
    (
      ~block: block,
      ~rules: list(rule),
      ~uhtyp: option(UHTyp.t), //due to currently unuse
      ~level: int,
    )
    : option(string) =>
  // block is the item to switch, rules are patterns, uhtyp is the last given type
  // case a | 1 => true | 2 => false end : bool
  // FIXME: still can let ocaml to do type inference, so just discard the uhtyp
  //        indeed, ocaml has nowhere to assign type for match structure
  //  Deal with the "?" holes, let x:Num = case... don't need a type at the end
  //      whether if the "?" hole itself is an incomplete expression with Holes
  switch (
    block_handler(~block, ~level),
    rule_handler(~rules, ~level=level + 1),
    uhtyp,
  ) {
  //FIXME: Currently using () to handle nested case, but a little bit ugly for the first one
  | (Some(b), Some(r), None) => Some("(match " ++ b ++ " with" ++ r ++ ")")
  | (Some(b), Some(r), Some(typ)) =>
    switch (uhtyp_translater(~t=typ)) {
    | None => None
    | Some(t) => Some("((match " ++ b ++ " with" ++ r ++ ") : " ++ t ++ ")")
    }
  // if uhtyp is not None, but translater result is None, it means there's incomplete hole, can't inference
  // FIXME: This is only a tricky method, so it isn't formally good
  | _ => None
  }
// expected to output "\n  | expr => expr "
and rule_handler = (~rules: list(rule), ~level: int): option(string) =>
  switch (rules) {
  | [] => Some("")
  | [rule, ...rest] =>
    switch (rule, rule_handler(~rules=rest, ~level)) {
    | (Rule(uhpat, block), Some(result)) =>
      switch (
        uhpat_translater(~t=uhpat),
        block_handler(~block, ~level=level + 1),
      ) {
      // Don't proceed on level because all are same level
      | (Some(t), Some(expr)) =>
        Some(
          "\n"
          ++ indent_space(~level)
          ++ "| "
          ++ t
          ++ " -> "
          ++ expr
          ++ result,
        )
      | _ => None
      }
    | _ => None
    }
  }
and lines_handler = (~lines: list(UHExp.line), ~level: int): option(string) =>
  switch (lines) {
  | [] => Some("")
  | [line, ...rest] =>
    switch (line_handler(~line, ~level), lines_handler(~lines=rest, ~level)) {
    | (Some(l), Some(r)) => Some(l ++ r)
    | _ => None
    }
  }

// we expect the every line will return a "\n" to end the line
and line_handler = (~line: UHExp.line, ~level: int): option(string) =>
  switch (line) {
  | ExpLine(t) =>
    option_string_concat(
      ~strs=[
        type_handler(~t, ~level),
        Some("\n"),
        Some(indent_space(~level)),
      ],
    )
  | EmptyLine => Some("\n")
  //let uhpat (: uhtyp) = block
  | LetLine(uhpat, uhtyp, block) =>
    switch (uhtyp) {
    | None =>
      option_string_concat(
        ~strs=[
          Some("let "),
          uhpat_translater(~t=uhpat),
          Some(" = "),
          block_handler(~block, ~level=level + 1),
          Some(" in\n"),
          Some(indent_space(~level)),
        ],
      )
    | Some(t) =>
      option_string_concat(
        ~strs=[
          Some("let "),
          uhpat_translater(~t=uhpat),
          Some(" : "),
          uhtyp_translater(~t),
          Some(" = "),
          block_handler(~block, ~level=level + 1),
          Some(" in\n"),
          Some(indent_space(~level)),
        ],
      )
    }
  };

//=============================

//used to handle None errors, or incomplete codes
let extraction_call = (~block: block): string =>
  switch (block_handler(~block, ~level=0)) {
  | None => "There could be some error in the code. Most possible is incomplete holes."
  | Some(s) => s
  };