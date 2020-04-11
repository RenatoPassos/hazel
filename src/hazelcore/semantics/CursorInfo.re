open Sexplib.Std;

[@deriving sexp]
type typed =
  // cursor is on a lambda with an argument type annotation
  /* cursor in analytic position */
  | AnaAnnotatedLambda(HTyp.t, HTyp.t)
  // cursor is on a type inconsistent expression
  | AnaTypeInconsistent(HTyp.t, HTyp.t)
  // cursor is on a tuple of the wrong length
  | AnaWrongLength
      // expected length
      (
        int,
        // got length
        int,
        // expected type
        HTyp.t,
      )
  // cursor is on a free variable
  | AnaFree(HTyp.t)
  // cursor is on a keyword
  | AnaKeyword(HTyp.t, ExpandingKeyword.t)
  // none of the above and didn't go through subsumption
  | Analyzed(HTyp.t)
  // none of the above and went through subsumption
  | AnaSubsumed(HTyp.t, HTyp.t) /* cursor in synthetic position */
  // cursor is on the function position of an ap,
  // and that expression does not synthesize a type
  // with a matched arrow type
  | SynErrorArrow
      // expected
      (
        HTyp.t,
        // got
        HTyp.t,
      )
  // cursor is on the function position of an ap,
  // and that expression does synthesize a type
  // with a matched arrow type
  | SynMatchingArrow(HTyp.t, HTyp.t)
  // cursor is on a free variable in the function
  // position of an ap
  | SynFreeArrow(HTyp.t)
  // cursor is on a keyword in the function position of an ap
  | SynKeywordArrow(HTyp.t, ExpandingKeyword.t)
  // none of the above, cursor is on a free variable
  | SynFree
  // cursor is on a keyword
  | SynKeyword(ExpandingKeyword.t)
  // none of the above
  | Synthesized(HTyp.t) /* cursor in analytic pattern position */
  // cursor is on a type inconsistent pattern
  | PatAnaTypeInconsistent(HTyp.t, HTyp.t)
  // cursor is on a tuple pattern of the wrong length
  | PatAnaWrongLength
      // expected length
      (
        int,
        // got length
        int,
        // expected type
        HTyp.t,
      )
  // cursor is on a keyword
  | PatAnaKeyword(HTyp.t, ExpandingKeyword.t)
  // none of the above and didn't go through subsumption
  | PatAnalyzed(HTyp.t)
  // none of the above and went through subsumption
  | PatAnaSubsumed(HTyp.t, HTyp.t) /* cursor in synthetic pattern position */
  // cursor is on a keyword
  | PatSynthesized(HTyp.t)
  | PatSynKeyword(ExpandingKeyword.t) /* cursor in type position */
  | OnType /* (we will have a richer structure here later)*/
  | OnLine
  | OnRule;

type cursor_term =
  | Exp(CursorPosition.t, UHExp.operand)
  | Pat(CursorPosition.t, UHPat.operand)
  | Typ(CursorPosition.t, UHTyp.operand)
  | ExpOp(CursorPosition.t, UHExp.operator)
  | PatOp(CursorPosition.t, UHPat.operator)
  | TypOp(CursorPosition.t, UHTyp.operator)
  | Line(CursorPosition.t, UHExp.line)
  | Rule(CursorPosition.t, UHExp.rule);
type outer_zexp =
  | ZExp(ZExp.t)
  | ZPat(ZPat.t)
  | ZTyp(ZTyp.t);
// TODO refactor into variants
// based on term sort and shape
//[@deriving sexp]
type t = {
  //term: cursor_term,
  typed,
  ctx: Contexts.t,
  // hack while merging
  uses: option(UsageAnalysis.uses_list),
};

type zoperand =
  | ZExp(ZExp.zoperand)
  | ZTyp(ZTyp.zoperand)
  | ZPat(ZPat.zoperand);

/* let rec get_outer_z_from_zexp = (zexp: ZExp.t): outer_zexp => {
     get_outer_z_from_zline(ZList.prj_z(zexp));
   }
   and get_outer_z_from_zline = (zline: ZExp.zline, outer_zexp): outer_zexp => {
     switch (zline) {
     | CursorL(_, _) => outer_zexp
     | ExpLineZ(zopseq) => get_outer_z_from_zexp_opseq(zopseq, outer_zexp)
     | LetLineZP(zpat, _, _) => get_outer_z_from_zpat(zpat)
     | LetLineZA(_, ztyp, _) => get_outer_z_from_ztyp(ztyp)
     | LetLineZE(_, _, zexp) => get_outer_z_from_zexp(zexp)
     };
   }
   and get_outer_z_from_zexp_opseq =
       (zopseq: ZExp.zopseq, outer_zexp): outer_zexp => {
     switch (zopseq) {
     | ZOpSeq(_, zseq) =>
       switch (zseq) {
       | ZOperand(zoperand, _) =>
         get_outer_z_from_zexp_operand(zoperand, outer_zexp)
       | ZOperator(_, _) => outer_zexp
       }
     };
   }
   and get_outer_z_from_zexp_operand =
       (zoperand: ZExp.zoperand, outer_zexp): outer_zexp => {
     switch (zoperand) {
     | CursorE(_, _) => outer_zexp
     | ParenthesizedZ(zexp) => get_outer_z_from_zexp(zexp)
     | LamZP(_, zpat, _, _) => get_outer_z_from_zpat(zpat)
     | LamZA(_, _, ztyp, _) => get_outer_z_from_ztyp(ztyp)
     | LamZE(_, _, _, zexp)
     | InjZ(_, _, zexp)
     | CaseZE(_, zexp, _, _) => get_outer_z_from_zexp(zexp)
     | CaseZR(_, _, zrules, _) => get_outer_z_from_zrules(zrules, outer_zexp)
     | CaseZA(_, _, _, ztyp) => get_outer_z_from_ztyp(ztyp)
     | ApPaletteZ(_, _, _, _) => failwith("not implemented")
     };
   }
   and get_outer_z_from_zrules = (zrules: ZExp.zrules, outer_zexp): outer_zexp => {
     get_outer_z_from_zrule(ZList.prj_z(zrules), outer_zexp);
   }
   and get_outer_z_from_zrule = (zrule: ZExp.zrule, outer_zexp): outer_zexp => {
     switch (zrule) {
     | CursorR(_, _) => outer_zexp
     | RuleZP(zpat, _) => get_outer_z_from_zpat(zpat)
     | RuleZE(_, zexp) => get_outer_z_from_zexp(zexp)
     };
   }
   and get_outer_z_from_zpat = (zpat: ZPat.t): outer_zexp => {
     get_outer_z_from_zpat_opseq(zpat, ZPat(zpat));
   }
   and get_outer_z_from_zpat_opseq =
       (zopseq: ZPat.zopseq, outer_zexp): outer_zexp => {
     switch (zopseq) {
     | ZOpSeq(_, zseq) =>
       switch (zseq) {
       | ZOperand(zpat_operand, _) =>
         get_outer_z_from_zpat_operand(zpat_operand, outer_zexp)
       | ZOperator(_, _) => outer_zexp
       }
     };
   }
   and get_outer_z_from_zpat_operand =
       (zoperand: ZPat.zoperand, outer_zexp): outer_zexp => {
     switch (zoperand) {
     | CursorP(_, _) => outer_zexp
     | ParenthesizedZ(zpat)
     | InjZ(_, _, zpat) => get_outer_z_from_zpat(zpat)
     };
   }
   and get_outer_z_from_ztyp = (ztyp: ZTyp.t): outer_zexp => {
     get_outer_z_from_ztyp_opseq(ztyp, ZTyp(ztyp));
   }
   and get_outer_z_from_ztyp_opseq =
       (zopseq: ZTyp.zopseq, outer_zexp): outer_zexp => {
     switch (zopseq) {
     | ZOpSeq(_, zseq) =>
       switch (zseq) {
       | ZOperand(ztyp_operand, _) =>
         get_outer_z_from_ztyp_operand(ztyp_operand, outer_zexp)
       | ZOperator(_, _) => outer_zexp
       }
     };
   }
   and get_outer_z_from_ztyp_operand =
       (zoperand: ZTyp.zoperand, outer_zexp): outer_zexp => {
     switch (zoperand) {
     | CursorT(_, _) => outer_zexp
     | ParenthesizedZ(ztyp)
     | ListZ(ztyp) => get_outer_z_from_ztyp(ztyp)
     };
   }; */

let rec get_zoperand = (zexp: ZExp.t): option(zoperand) => {
  get_zoperand_from_zexp(zexp);
}
and get_zoperand_from_zexp = (zexp: ZExp.t): option(zoperand) => {
  get_zoperand_from_zline(ZList.prj_z(zexp));
}
and get_zoperand_from_zline = (zline: ZExp.zline): option(zoperand) => {
  switch (zline) {
  | CursorL(_, _) => None
  | ExpLineZ(zopseq) => get_zoperand_from_zexp_opseq(zopseq)
  | LetLineZP(zpat, _, _) => get_zoperand_from_zpat(zpat)
  | LetLineZA(_, ztyp, _) => get_zoperand_from_ztyp(ztyp)
  | LetLineZE(_, _, zexp) => get_zoperand_from_zexp(zexp)
  };
}
and get_zoperand_from_zexp_opseq = (zopseq: ZExp.zopseq): option(zoperand) => {
  switch (zopseq) {
  | ZOpSeq(_, zseq) =>
    switch (zseq) {
    | ZOperand(zoperand, _) => get_zoperand_from_zexp_operand(zoperand)
    | ZOperator(_, _) => None
    }
  };
}
and get_zoperand_from_zexp_operand =
    (zoperand: ZExp.zoperand): option(zoperand) => {
  switch (zoperand) {
  | CursorE(_, _) => Some(ZExp(zoperand))
  | ParenthesizedZ(zexp) => get_zoperand_from_zexp(zexp)
  | LamZP(_, zpat, _, _) => get_zoperand_from_zpat(zpat)
  | LamZA(_, _, ztyp, _) => get_zoperand_from_ztyp(ztyp)
  | LamZE(_, _, _, zexp)
  | InjZ(_, _, zexp)
  | CaseZE(_, zexp, _, _) => get_zoperand_from_zexp(zexp)
  | CaseZR(_, _, zrules, _) => get_zoperand_from_zrules(zrules)
  | CaseZA(_, _, _, ztyp) => get_zoperand_from_ztyp(ztyp)
  | ApPaletteZ(_, _, _, _) => failwith("not implemented")
  };
}
and get_zoperand_from_zrules = (zrules: ZExp.zrules): option(zoperand) => {
  get_zoperand_from_zrule(ZList.prj_z(zrules));
}
and get_zoperand_from_zrule = (zrule: ZExp.zrule): option(zoperand) => {
  switch (zrule) {
  | CursorR(_, _) => None
  | RuleZP(zpat, _) => get_zoperand_from_zpat(zpat)
  | RuleZE(_, zexp) => get_zoperand_from_zexp(zexp)
  };
}
and get_zoperand_from_zpat = (zpat: ZPat.t): option(zoperand) => {
  get_zoperand_from_zpat_opseq(zpat);
}
and get_zoperand_from_zpat_opseq = (zopseq: ZPat.zopseq): option(zoperand) => {
  switch (zopseq) {
  | ZOpSeq(_, zseq) =>
    switch (zseq) {
    | ZOperand(zpat_operand, _) =>
      get_zoperand_from_zpat_operand(zpat_operand)
    | ZOperator(_, _) => None
    }
  };
}
and get_zoperand_from_zpat_operand =
    (zoperand: ZPat.zoperand): option(zoperand) => {
  switch (zoperand) {
  | CursorP(_, _) => Some(ZPat(zoperand))
  | ParenthesizedZ(zpat)
  | InjZ(_, _, zpat) => get_zoperand_from_zpat(zpat)
  };
}
and get_zoperand_from_ztyp = (ztyp: ZTyp.t): option(zoperand) => {
  get_zoperand_from_ztyp_opseq(ztyp);
}
and get_zoperand_from_ztyp_opseq = (zopseq: ZTyp.zopseq): option(zoperand) => {
  switch (zopseq) {
  | ZOpSeq(_, zseq) =>
    switch (zseq) {
    | ZOperand(ztyp_operand, _) =>
      get_zoperand_from_ztyp_operand(ztyp_operand)
    | ZOperator(_, _) => None
    }
  };
}
and get_zoperand_from_ztyp_operand =
    (zoperand: ZTyp.zoperand): option(zoperand) => {
  switch (zoperand) {
  | CursorT(_, _) => Some(ZTyp(zoperand))
  | ParenthesizedZ(ztyp)
  | ListZ(ztyp) => get_zoperand_from_ztyp(ztyp)
  };
};

let caret_is_after_zoperand = (zexp: ZExp.t): bool => {
  switch (get_zoperand(zexp)) {
  | None => false
  | Some(zop) =>
    switch (zop) {
    | ZExp(zoperand) => ZExp.is_after_zoperand(zoperand)
    | ZPat(zoperand) => ZPat.is_after_zoperand(zoperand)
    | ZTyp(zoperand) => ZTyp.is_after_zoperand(zoperand)
    }
  };
};
let caret_is_before_zoperand = (zexp: ZExp.t): bool => {
  switch (get_zoperand(zexp)) {
  | None => false
  | Some(zop) =>
    switch (zop) {
    | ZExp(zoperand) => ZExp.is_before_zoperand(zoperand)
    | ZPat(zoperand) => ZPat.is_before_zoperand(zoperand)
    | ZTyp(zoperand) => ZTyp.is_before_zoperand(zoperand)
    }
  };
};
let rec extract_cursor_term = (exp: ZExp.t): (cursor_term, bool, bool) => {
  let cursor_term = extract_cursor_exp_term(exp);
  let prev_is_empty_line = {
    let prefix = ZList.prj_prefix(exp);
    switch (ListUtil.split_last(prefix)) {
    | None => false
    | Some((_, elt)) => UHExp.is_empty_line(elt)
    };
  };
  let next_is_empty_line = {
    let suffix = ZList.prj_suffix(exp);
    switch (suffix) {
    | [] => false
    | ls => UHExp.is_empty_line(List.hd(ls))
    };
  };
  (cursor_term, prev_is_empty_line, next_is_empty_line);
}
and extract_cursor_exp_term = (exp: ZExp.t): cursor_term => {
  extract_from_zline(ZList.prj_z(exp));
}
and extract_from_zline = (zline: ZExp.zline): cursor_term => {
  switch (zline) {
  | CursorL(cursor_pos, uex_line) => Line(cursor_pos, uex_line)
  | ExpLineZ(zopseq) => extract_from_zexp_opseq(zopseq)
  | LetLineZP(zpat, _, _) => extract_cursor_pat_term(zpat)
  | LetLineZA(_, ztyp, _) => extract_cursor_type_term(ztyp)
  | LetLineZE(_, _, zexp) => extract_cursor_exp_term(zexp)
  };
}
and extract_from_zexp_operand = (zexp_operand: ZExp.zoperand): cursor_term => {
  switch (zexp_operand) {
  | CursorE(cursor_pos, operand) => Exp(cursor_pos, operand)
  | ParenthesizedZ(zexp) => extract_cursor_exp_term(zexp)
  | LamZP(_, zpat, _, _) => extract_cursor_pat_term(zpat)
  | LamZA(_, _, ztyp, _) => extract_cursor_type_term(ztyp)
  | LamZE(_, _, _, zexp)
  | InjZ(_, _, zexp)
  | CaseZE(_, zexp, _, _) => extract_cursor_exp_term(zexp)
  | CaseZR(_, _, zrules, _) => extract_from_zrules(zrules)
  | CaseZA(_, _, _, ztyp) => extract_cursor_type_term(ztyp)
  | ApPaletteZ(_, _, _, _) => failwith("ApPalette is not implemented")
  };
}
and extract_from_zrules = (zrules: ZExp.zrules): cursor_term => {
  let zrule = ZList.prj_z(zrules);
  switch (zrule) {
  | CursorR(cursor_pos, uex_rule) => Rule(cursor_pos, uex_rule)
  | RuleZP(zpat, _) => extract_cursor_pat_term(zpat)
  | RuleZE(_, zexp) => extract_cursor_exp_term(zexp)
  };
}
and extract_from_zexp_opseq = (zopseq: ZExp.zopseq): cursor_term => {
  switch (zopseq) {
  | ZOpSeq(_, zseq) =>
    switch (zseq) {
    | ZOperand(zoperand, _) => extract_from_zexp_operand(zoperand)
    | ZOperator(zoperator, _) =>
      let (cursor_pos, uop) = zoperator;
      ExpOp(cursor_pos, uop);
    }
  };
}
and extract_cursor_pat_term = (zpat: ZPat.t): cursor_term => {
  switch (zpat) {
  | ZOpSeq(_, zseq) =>
    switch (zseq) {
    | ZOperand(zpat_operand, _) => extract_from_zpat_operand(zpat_operand)
    | ZOperator(zpat_operator, _) =>
      let (cursor_pos, uop) = zpat_operator;
      PatOp(cursor_pos, uop);
    }
  };
}
and extract_from_zpat_operand = (zpat_operand: ZPat.zoperand): cursor_term => {
  switch (zpat_operand) {
  | CursorP(cursor_pos, upat_operand) => Pat(cursor_pos, upat_operand)
  | ParenthesizedZ(zpat)
  | InjZ(_, _, zpat) => extract_cursor_pat_term(zpat)
  };
}
and extract_cursor_type_term = (ztyp: ZTyp.t): cursor_term => {
  switch (ztyp) {
  | ZOpSeq(_, zseq) =>
    switch (zseq) {
    | ZOperand(ztyp_operand, _) => extract_from_ztyp_operand(ztyp_operand)
    | ZOperator(ztyp_operator, _) =>
      let (cursor_pos, uop) = ztyp_operator;
      TypOp(cursor_pos, uop);
    }
  };
}
and extract_from_ztyp_operand = (ztyp_operand: ZTyp.zoperand): cursor_term => {
  switch (ztyp_operand) {
  | CursorT(cursor_pos, utyp_operand) => Typ(cursor_pos, utyp_operand)
  | ParenthesizedZ(ztyp)
  | ListZ(ztyp) => extract_cursor_type_term(ztyp)
  };
};

let is_hole = (cursor_term: cursor_term): bool => {
  switch (cursor_term) {
  | Exp(_, exp) => UHExp.operand_is_hole(exp)
  | Pat(_, pat) => UHPat.operand_is_hole(pat)
  | Typ(_, typ) => UHTyp.operand_is_hole(typ)
  | ExpOp(_, _)
  | PatOp(_, _)
  | TypOp(_, _)
  | Line(_, _)
  | Rule(_, _) => false
  };
};

let is_empty_line = (cursor_term): bool => {
  switch (cursor_term) {
  | Line(_, line) => UHExp.is_empty_line(line)
  | Exp(_, _)
  | Pat(_, _)
  | Typ(_, _)
  | ExpOp(_, _)
  | PatOp(_, _)
  | TypOp(_, _)
  | Rule(_, _) => false
  };
};

let get_cursor_pos = (cursor_term: cursor_term) => {
  switch (cursor_term) {
  | Exp(cursor_pos, _)
  | Pat(cursor_pos, _)
  | Typ(cursor_pos, _)
  | ExpOp(cursor_pos, _)
  | PatOp(cursor_pos, _)
  | TypOp(cursor_pos, _)
  | Line(cursor_pos, _)
  | Rule(cursor_pos, _) => cursor_pos
  };
};
let mk = (~uses=?, typed, ctx) => {typed, ctx, uses};

let get_ctx = ci => ci.ctx;

module Typ = {
  let cursor_info = (~steps as _, ctx: Contexts.t, _: ZTyp.t): option(t) =>
    Some(mk(OnType, ctx));
} /*
 * there are cases we can't determine where to find the uses of a variable
 * immediately after we see its binding site.
 * in this case, we will return a deferrable('t) and go up the tree
 * until we could find uses and feed it to (uses_list => 't).
 */;

type deferrable('t) =
  | CursorNotOnDeferredVarPat('t)
  | CursorOnDeferredVarPat(UsageAnalysis.uses_list => 't, Var.t);

module Pat = {
  let rec syn_cursor_info =
          (~steps=[], ctx: Contexts.t, zp: ZPat.t): option(deferrable(t)) =>
    syn_cursor_info_zopseq(~steps, ctx, zp)
  and syn_cursor_info_zopseq =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        ZOpSeq(skel, zseq): ZPat.zopseq,
      )
      : option(deferrable(t)) => {
    // handle n-tuples:
    // cannot simply defer to syn_cursor_info_skel here
    // because it assumes binary tupling -- this would
    // cause sub-tuples to synthesize sub-product types,
    // but we want all comma operators in an opseq to
    // show the complete product type
    let seq = zseq |> ZPat.erase_zseq;
    let skels = skel |> UHPat.get_tuple_elements;
    switch (zseq) {
    | ZOperator((_, Comma), _) =>
      // cursor on tuple comma
      skels
      |> List.fold_left(
           (acc: option((list(HTyp.t), Contexts.t)), skel) =>
             switch (acc) {
             | None => None
             | Some((rev_tys, ctx)) =>
               switch (Statics.Pat.syn_skel(ctx, skel, seq)) {
               | None => None
               | Some((ty, ctx)) => Some(([ty, ...rev_tys], ctx))
               }
             },
           Some(([], ctx)),
         )
      |> OptUtil.map(((rev_tys, _)) =>
           CursorNotOnDeferredVarPat(
             mk(PatSynthesized(rev_tys |> List.rev |> HTyp.make_tuple), ctx),
           )
         )
    | _ =>
      // cursor within tuple element
      let opt_ctx =
        skels
        |> ListUtil.take_while(skel =>
             !ZOpSeq.skel_contains_cursor(skel, zseq)
           )
        |> List.fold_left(
             (opt_ctx, skel) =>
               switch (opt_ctx) {
               | None => None
               | Some(ctx) =>
                 Statics.Pat.syn_skel(ctx, skel, seq)
                 |> Option.map(((_, ctx)) => ctx)
               },
             Some(ctx),
           );
      switch (opt_ctx) {
      | None => None
      | Some(ctx) =>
        let cursor_skel =
          skels |> List.find(skel => ZOpSeq.skel_contains_cursor(skel, zseq));
        syn_cursor_info_skel(~steps, ctx, cursor_skel, zseq);
      };
    };
  }
  and syn_cursor_info_skel =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        skel: UHPat.skel,
        zseq: ZPat.zseq,
      )
      : option(deferrable(t)) => {
    let seq = zseq |> ZPat.erase_zseq;
    if (ZOpSeq.skel_is_rooted_at_cursor(skel, zseq)) {
      // found cursor
      switch (zseq) {
      | ZOperand(zoperand, (prefix, _)) =>
        syn_cursor_info_zoperand(
          ~steps=steps @ [Seq.length_of_affix(prefix)],
          ctx,
          zoperand,
        )
      | ZOperator(_) =>
        Statics.Pat.syn_skel(ctx, skel, seq)
        |> OptUtil.map(((ty, _)) =>
             CursorNotOnDeferredVarPat(mk(PatSynthesized(ty), ctx))
           )
      };
    } else {
      // recurse toward cursor
      switch (skel) {
      | Placeholder(_) => None
      | BinOp(_, Comma, _, _) =>
        failwith(
          "Pat.syn_cursor_info_skel: expected commas to be handled at opseq level",
        )
      | BinOp(_, Space, skel1, skel2) =>
        switch (ana_cursor_info_skel(~steps, ctx, skel1, zseq, HTyp.Hole)) {
        | Some(_) as res => res
        | None =>
          switch (Statics.Pat.ana_skel(ctx, skel1, seq, Hole)) {
          | None => None
          | Some(ctx) => ana_cursor_info_skel(~steps, ctx, skel2, zseq, Hole)
          }
        }
      | BinOp(_, Cons, skel1, skel2) =>
        switch (syn_cursor_info_skel(~steps, ctx, skel1, zseq)) {
        | Some(_) as res => res
        | None =>
          switch (Statics.Pat.syn_skel(ctx, skel1, seq)) {
          | None => None
          | Some((ty_elt, ctx)) =>
            ana_cursor_info_skel(~steps, ctx, skel2, zseq, HTyp.List(ty_elt))
          }
        }
      };
    };
  }
  and syn_cursor_info_zoperand =
      (~steps: CursorPath.steps, ctx: Contexts.t, zoperand: ZPat.zoperand)
      : option(deferrable(t)) =>
    switch (zoperand) {
    | CursorP(_, Var(_, InVarHole(Keyword(k), _), _)) =>
      Some(CursorNotOnDeferredVarPat(mk(PatSynKeyword(k), ctx)))
    | CursorP(_, Var(NotInHole, NotInVarHole, x) as p) =>
      Statics.Pat.syn_operand(ctx, p)
      |> OptUtil.map(((ty, _)) =>
           CursorOnDeferredVarPat(
             uses => mk(~uses, PatSynthesized(ty), ctx),
             x,
           )
         )
    | CursorP(_, p) =>
      Statics.Pat.syn_operand(ctx, p)
      |> OptUtil.map(((ty, _)) =>
           CursorNotOnDeferredVarPat(mk(PatSynthesized(ty), ctx))
         )
    | InjZ(_, _, zbody)
    | ParenthesizedZ(zbody) =>
      syn_cursor_info(~steps=steps @ [0], ctx, zbody)
    }
  and ana_cursor_info =
      (~steps, ctx: Contexts.t, zp: ZPat.t, ty: HTyp.t)
      : option(deferrable(t)) => {
    ana_cursor_info_zopseq(~steps, ctx, zp, ty);
  }
  and ana_cursor_info_zopseq =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        ZOpSeq(skel, zseq) as zopseq: ZPat.zopseq,
        ty: HTyp.t,
      )
      : option(deferrable(t)) => {
    // handle n-tuples:
    // cannot simply defer to ana_cursor_info_skel here
    // because it assumes binary tupling -- this would
    // cause sub-tuples to synthesize sub-product types,
    // but we want all comma operators in an opseq to
    // show the complete product type
    let seq = zseq |> ZPat.erase_zseq;
    let skel_tys = Statics.Pat.tuple_zip(skel, ty);
    switch (zseq) {
    | ZOperator((_, Comma), _) =>
      // cursor on tuple comma
      switch (skel_tys) {
      | Some(_) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnalyzed(ty), ctx)))
      | None =>
        let expected_length = ty |> HTyp.get_prod_elements |> List.length;
        let got_length = skel |> UHPat.get_tuple_elements |> List.length;
        Some(
          CursorNotOnDeferredVarPat(
            mk(PatAnaWrongLength(expected_length, got_length, ty), ctx),
          ),
        );
      }
    | _ =>
      // cursor in tuple element
      switch (skel_tys) {
      | None =>
        // wrong length, switch to syn
        let zopseq_not_in_hole =
          zopseq |> ZPat.set_err_status_zopseq(NotInHole);
        syn_cursor_info_zopseq(~steps, ctx, zopseq_not_in_hole);
      | Some(skel_tys) =>
        let opt_ctx =
          skel_tys
          |> ListUtil.take_while(((skel, _)) =>
               !ZOpSeq.skel_contains_cursor(skel, zseq)
             )
          |> List.fold_left(
               (opt_ctx, (skel, ty)) =>
                 switch (opt_ctx) {
                 | None => None
                 | Some(ctx) => Statics.Pat.ana_skel(ctx, skel, seq, ty)
                 },
               Some(ctx),
             );
        switch (opt_ctx) {
        | None => None
        | Some(ctx) =>
          let (cursor_skel, ty) =
            skel_tys
            |> List.find(((skel, _)) =>
                 ZOpSeq.skel_contains_cursor(skel, zseq)
               );
          ana_cursor_info_skel(~steps, ctx, cursor_skel, zseq, ty);
        };
      }
    };
  }
  and ana_cursor_info_skel =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        skel: UHPat.skel,
        zseq: ZPat.zseq,
        ty: HTyp.t,
      )
      : option(deferrable(t)) => {
    let seq = zseq |> ZPat.erase_zseq;
    if (ZOpSeq.skel_is_rooted_at_cursor(skel, zseq)) {
      // found cursor
      switch (zseq) {
      | ZOperand(zoperand, (prefix, _)) =>
        ana_cursor_info_zoperand(
          ~steps=steps @ [Seq.length_of_affix(prefix)],
          ctx,
          zoperand,
          ty,
        )
      | ZOperator(_) =>
        Statics.Pat.ana_skel(ctx, skel, seq, ty)
        |> OptUtil.map(_ =>
             CursorNotOnDeferredVarPat(mk(PatAnalyzed(ty), ctx))
           )
      };
    } else {
      // recurse toward cursor
      switch (skel) {
      | Placeholder(_) => None
      | BinOp(InHole(_), _, _, _) =>
        syn_cursor_info_skel(~steps, ctx, skel, zseq)
      | BinOp(_, Comma, _, _) =>
        failwith(
          "Pat.ana_cursor_info_skel: expected commas to be handled at opseq level",
        )
      | BinOp(NotInHole, Space, skel1, skel2) =>
        switch (ana_cursor_info_skel(~steps, ctx, skel1, zseq, Hole)) {
        | Some(_) as res => res
        | None =>
          switch (Statics.Pat.ana_skel(ctx, skel1, seq, Hole)) {
          | None => None
          | Some(ctx) => ana_cursor_info_skel(~steps, ctx, skel2, zseq, Hole)
          }
        }
      | BinOp(NotInHole, Cons, skel1, skel2) =>
        switch (HTyp.matched_list(ty)) {
        | None => None
        | Some(ty_elt) =>
          switch (ana_cursor_info_skel(~steps, ctx, skel1, zseq, ty_elt)) {
          | Some(_) as res => res
          | None =>
            switch (Statics.Pat.ana_skel(ctx, skel1, seq, ty_elt)) {
            | None => None
            | Some(ctx) =>
              ana_cursor_info_skel(~steps, ctx, skel2, zseq, List(ty_elt))
            }
          }
        }
      };
    };
  }
  and ana_cursor_info_zoperand =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        zoperand: ZPat.zoperand,
        ty: HTyp.t,
      )
      : option(deferrable(t)) =>
    switch (zoperand) {
    | CursorP(_, operand) =>
      switch (operand) {
      // in hole
      | EmptyHole(_) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnaSubsumed(ty, Hole), ctx)))
      | Wild(InHole(TypeInconsistent, _))
      | Var(InHole(TypeInconsistent, _), _, _)
      | NumLit(InHole(TypeInconsistent, _), _)
      | BoolLit(InHole(TypeInconsistent, _), _)
      | ListNil(InHole(TypeInconsistent, _))
      | Inj(InHole(TypeInconsistent, _), _, _) =>
        let operand' = UHPat.set_err_status_operand(NotInHole, operand);
        switch (Statics.Pat.syn_operand(ctx, operand')) {
        | None => None
        | Some((ty', _)) =>
          Some(
            CursorNotOnDeferredVarPat(
              mk(PatAnaTypeInconsistent(ty, ty'), ctx),
            ),
          )
        };
      | Wild(InHole(WrongLength, _))
      | Var(InHole(WrongLength, _), _, _)
      | NumLit(InHole(WrongLength, _), _)
      | BoolLit(InHole(WrongLength, _), _)
      | ListNil(InHole(WrongLength, _))
      | Inj(InHole(WrongLength, _), _, _) => None
      | Var(NotInHole, InVarHole(Keyword(k), _), _) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnaKeyword(ty, k), ctx)))
      // not in hole
      | Var(NotInHole, _, x) =>
        Some(
          CursorOnDeferredVarPat(
            uses => mk(~uses, PatAnalyzed(ty), ctx),
            x,
          ),
        )
      | Wild(NotInHole)
      | ListNil(NotInHole) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnalyzed(ty), ctx)))
      | NumLit(NotInHole, _) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnaSubsumed(ty, Num), ctx)))
      | BoolLit(NotInHole, _) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnaSubsumed(ty, Bool), ctx)))
      | Inj(NotInHole, _, _) =>
        Some(CursorNotOnDeferredVarPat(mk(PatAnalyzed(ty), ctx)))
      | Parenthesized(body) =>
        Statics.Pat.ana(ctx, body, ty)
        |> OptUtil.map(_ =>
             CursorNotOnDeferredVarPat(mk(PatAnalyzed(ty), ctx))
           )
      }
    | InjZ(InHole(WrongLength, _), _, _) => None
    | InjZ(InHole(TypeInconsistent, _), _, _) =>
      syn_cursor_info_zoperand(~steps, ctx, zoperand)
    | InjZ(NotInHole, position, zbody) =>
      switch (HTyp.matched_sum(ty)) {
      | None => None
      | Some((tyL, tyR)) =>
        let ty_body = InjSide.pick(position, tyL, tyR);
        ana_cursor_info(~steps=steps @ [0], ctx, zbody, ty_body);
      }
    | ParenthesizedZ(zbody) =>
      ana_cursor_info(~steps=steps @ [0], ctx, zbody, ty)
    };
};

module Exp = {
  let rec cursor_on_outer_expr:
    ZExp.zoperand => option((ErrStatus.t, VarErrStatus.t)) =
    fun
    | CursorE(_, operand) => {
        let err = operand |> UHExp.get_err_status_operand;
        let verr =
          switch (operand) {
          | Var(_, verr, _) => verr
          | _ => NotInVarHole
          };
        Some((err, verr));
      }
    | ParenthesizedZ(([], ExpLineZ(ZOpSeq(skel, zseq)), [])) =>
      if (ZOpSeq.skel_is_rooted_at_cursor(skel, zseq)) {
        switch (skel, zseq) {
        | (BinOp(err, _, _, _), _) => Some((err, NotInVarHole))
        | (_, ZOperand(zoperand, _)) => cursor_on_outer_expr(zoperand)
        | _ => None
        };
      } else {
        None;
      }
    | _ => None;

  let rec syn_cursor_info =
          (~steps=[], ctx: Contexts.t, ze: ZExp.t): option(t) => {
    syn_cursor_info_zblock(~steps, ctx, ze);
  }
  and syn_cursor_info_zblock =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        (prefix, zline, suffix): ZExp.zblock,
      )
      : option(t) =>
    switch (Statics.Exp.syn_lines(ctx, prefix)) {
    | None => None
    | Some(ctx) =>
      switch (
        syn_cursor_info_line(
          ~steps=steps @ [List.length(prefix)],
          ctx,
          zline,
        )
      ) {
      | None => None
      | Some(CursorNotOnDeferredVarPat(ci)) => Some(ci)
      | Some(CursorOnDeferredVarPat(deferred_ci, x)) =>
        let uses =
          UsageAnalysis.find_uses_block(
            ~offset=List.length(prefix) + 1,
            ~steps,
            x,
            suffix,
          );
        Some(uses |> deferred_ci);
      }
    }
  and syn_cursor_info_line =
      (~steps: CursorPath.steps, ctx: Contexts.t, zline: ZExp.zline)
      : option(deferrable(t)) =>
    switch (zline) {
    | CursorL(_) => Some(CursorNotOnDeferredVarPat(mk(OnLine, ctx)))
    | ExpLineZ(ze) =>
      switch (syn_cursor_info_zopseq(~steps, ctx, ze)) {
      | None => None
      | Some(ci) => Some(CursorNotOnDeferredVarPat(ci))
      }
    | LetLineZP(zp, ann, def) =>
      let pat_ci = Pat.ana_cursor_info(~steps=steps @ [0], ctx, zp);
      switch (ann) {
      | None =>
        switch (Statics.Exp.syn(ctx, def)) {
        | None => None
        | Some(ty1) => pat_ci(ty1)
        }
      | Some(ann) =>
        let ty1 = ann |> UHTyp.expand;
        switch (pat_ci(ty1)) {
        | None => None
        | Some(CursorNotOnDeferredVarPat(_)) as deferrable => deferrable
        | Some(CursorOnDeferredVarPat(deferred, x)) as deferrable =>
          switch (HTyp.matched_arrow(ty1)) {
          | None => deferrable
          | Some(_) =>
            let rec_uses =
              UsageAnalysis.find_uses(~steps=steps @ [2], x, def);
            Some(
              CursorOnDeferredVarPat(uses => rec_uses @ uses |> deferred, x),
            );
          }
        };
      };
    | LetLineZA(_, zann, _) =>
      Typ.cursor_info(~steps=steps @ [1], ctx, zann)
      |> OptUtil.map(ci => CursorNotOnDeferredVarPat(ci))
    | LetLineZE(p, ann, zdef) =>
      switch (ann) {
      | None =>
        syn_cursor_info(~steps=steps @ [2], ctx, zdef)
        |> OptUtil.map(ci => CursorNotOnDeferredVarPat(ci))
      | Some(ann) =>
        let ty = UHTyp.expand(ann);
        let ctx_def = Statics.Exp.ctx_for_let(ctx, p, ty, zdef |> ZExp.erase);
        ana_cursor_info(~steps=steps @ [2], ctx_def, zdef, ty)
        |> OptUtil.map(ci => CursorNotOnDeferredVarPat(ci));
      }
    }
  and syn_cursor_info_zopseq =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        ZOpSeq(skel, zseq) as zopseq: ZExp.zopseq,
      )
      : option(t) => {
    // handle n-tuples:
    // cannot simply defer to syn_cursor_info_skel here
    // because it assumes binary tupling -- this would
    // cause sub-tuples to synthesize sub-product types,
    // but we want all comma operators in an opseq to
    // show the complete product type
    switch (zseq) {
    | ZOperator((_, Comma), _) =>
      // cursor on tuple comma
      Statics.Exp.syn_opseq(ctx, zopseq |> ZExp.erase_zopseq)
      |> OptUtil.map(ty => mk(Synthesized(ty), ctx))
    | _ =>
      // cursor within tuple element
      let cursor_skel =
        skel
        |> UHExp.get_tuple_elements
        |> List.find(skel => ZOpSeq.skel_contains_cursor(skel, zseq));
      syn_cursor_info_skel(~steps, ctx, cursor_skel, zseq);
    };
  }
  and syn_cursor_info_skel =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        skel: UHExp.skel,
        zseq: ZExp.zseq,
      )
      : option(t) => {
    let seq = zseq |> ZExp.erase_zseq;
    if (ZOpSeq.skel_is_rooted_at_cursor(skel, zseq)) {
      // found cursor
      switch (zseq) {
      | ZOperand(zoperand, (prefix, _)) =>
        syn_cursor_info_zoperand(
          ~steps=steps @ [Seq.length_of_affix(prefix)],
          ctx,
          zoperand,
        )
      | ZOperator(_) =>
        Statics.Exp.syn_skel(ctx, skel, seq)
        |> OptUtil.map(ty => mk(Synthesized(ty), ctx))
      };
    } else {
      // recurse toward cursor
      switch (skel) {
      | Placeholder(_) => None
      | BinOp(_, Comma, _, _) =>
        failwith(
          "Exp.syn_cursor_info_skel: expected commas to be handled at opseq level",
        )
      | BinOp(
          _,
          Minus | Plus | Times | LessThan | GreaterThan | Equals,
          skel1,
          skel2,
        ) =>
        switch (ana_cursor_info_skel(~steps, ctx, skel1, zseq, HTyp.Num)) {
        | Some(_) as result => result
        | None => ana_cursor_info_skel(~steps, ctx, skel2, zseq, Num)
        }
      | BinOp(_, And | Or, skel1, skel2) =>
        switch (ana_cursor_info_skel(~steps, ctx, skel1, zseq, Bool)) {
        | Some(_) as result => result
        | None => ana_cursor_info_skel(~steps, ctx, skel2, zseq, Bool)
        }
      | BinOp(_, Space, Placeholder(n) as skel1, skel2) =>
        if (ZOpSeq.skel_contains_cursor(skel1, zseq)) {
          let zoperand =
            switch (zseq) {
            | ZOperator(_) => assert(false)
            | ZOperand(zoperand, _) => zoperand
            };
          let mk = typed => mk(typed, ctx);
          switch (cursor_on_outer_expr(zoperand)) {
          | None =>
            syn_cursor_info_zoperand(~steps=steps @ [n], ctx, zoperand)
          | Some((InHole(WrongLength, _), _)) => None
          | Some((InHole(TypeInconsistent, _), _)) =>
            let operand_nih =
              zoperand
              |> ZExp.erase_zoperand
              |> UHExp.set_err_status_operand(NotInHole);
            Statics.Exp.syn_operand(ctx, operand_nih)
            |> OptUtil.map(ty => mk(SynErrorArrow(Arrow(Hole, Hole), ty)));
          | Some((_, InVarHole(Free, _))) =>
            Some(mk(SynFreeArrow(Arrow(Hole, Hole))))
          | Some((_, InVarHole(Keyword(k), _))) =>
            Some(mk(SynKeywordArrow(Arrow(Hole, Hole), k)))
          | Some((NotInHole, NotInVarHole)) =>
            switch (
              Statics.Exp.syn_operand(ctx, zoperand |> ZExp.erase_zoperand)
            ) {
            | None => None
            | Some(ty) =>
              HTyp.matched_arrow(ty)
              |> OptUtil.map(((ty1, ty2)) =>
                   mk(SynMatchingArrow(ty, Arrow(ty1, ty2)))
                 )
            }
          };
        } else {
          switch (Statics.Exp.syn_skel(ctx, skel1, seq)) {
          | None => None
          | Some(ty) =>
            switch (HTyp.matched_arrow(ty)) {
            | None => None
            | Some((ty1, _)) =>
              ana_cursor_info_skel(~steps, ctx, skel2, zseq, ty1)
            }
          };
        }
      | BinOp(_, Space, skel1, skel2) =>
        switch (syn_cursor_info_skel(~steps, ctx, skel1, zseq)) {
        | Some(_) as result => result
        | None =>
          switch (Statics.Exp.syn_skel(ctx, skel1, seq)) {
          | None => None
          | Some(ty) =>
            switch (HTyp.matched_arrow(ty)) {
            | None => None
            | Some((ty1, _)) =>
              ana_cursor_info_skel(~steps, ctx, skel2, zseq, ty1)
            }
          }
        }
      | BinOp(_, Cons, skel1, skel2) =>
        switch (syn_cursor_info_skel(~steps, ctx, skel1, zseq)) {
        | Some(_) as result => result
        | None =>
          switch (Statics.Exp.syn_skel(ctx, skel1, seq)) {
          | None => None
          | Some(ty_elt) =>
            let ty_list = HTyp.List(ty_elt);
            ana_cursor_info_skel(~steps, ctx, skel2, zseq, ty_list);
          }
        }
      };
    };
  }
  and syn_cursor_info_zoperand =
      (~steps: CursorPath.steps, ctx: Contexts.t, zoperand: ZExp.zoperand)
      : option(t) => {
    switch (zoperand) {
    | CursorE(_, Var(_, InVarHole(Keyword(k), _), _)) =>
      Some(mk(SynKeyword(k), ctx))
    | CursorE(_, Var(_, InVarHole(Free, _), _)) => Some(mk(SynFree, ctx))
    | CursorE(_, e) =>
      switch (Statics.Exp.syn_operand(ctx, e)) {
      | None => None
      | Some(ty) => Some(mk(Synthesized(ty), ctx))
      }
    | ParenthesizedZ(zbody) =>
      syn_cursor_info(~steps=steps @ [0], ctx, zbody)
    | LamZP(_, zp, ann, body) =>
      let ty1 =
        switch (ann) {
        | None => HTyp.Hole
        | Some(uty1) => UHTyp.expand(uty1)
        };
      switch (Pat.ana_cursor_info(~steps=steps @ [0], ctx, zp, ty1)) {
      | None => None
      | Some(CursorNotOnDeferredVarPat(ci)) => Some(ci)
      | Some(CursorOnDeferredVarPat(deferred_ci, x)) =>
        let uses = UsageAnalysis.find_uses(~steps=steps @ [2], x, body);
        Some(uses |> deferred_ci);
      };
    | LamZA(_, _, zann, _) => Typ.cursor_info(~steps=steps @ [1], ctx, zann)
    | LamZE(_, p, ann, zbody) =>
      let ty1 =
        switch (ann) {
        | Some(uty1) => UHTyp.expand(uty1)
        | None => Hole
        };
      switch (Statics.Pat.ana(ctx, p, ty1)) {
      | None => None
      | Some(ctx1) => syn_cursor_info(~steps=steps @ [2], ctx1, zbody)
      };
    | InjZ(_, _, zbody) => syn_cursor_info(~steps=steps @ [0], ctx, zbody)
    | CaseZE(_, _, _, None)
    | CaseZR(_, _, _, None) => None
    | CaseZE(_, zscrut, _, Some(_)) =>
      syn_cursor_info(~steps=steps @ [0], ctx, zscrut)
    | CaseZR(_, scrut, (prefix, zrule, _), Some(ann)) =>
      let clause_ty = UHTyp.expand(ann);
      switch (Statics.Exp.syn(ctx, scrut)) {
      | None => None
      | Some(pat_ty) =>
        ana_cursor_info_rule(
          ~steps=steps @ [1 + List.length(prefix)],
          ctx,
          zrule,
          pat_ty,
          clause_ty,
        )
      };
    | CaseZA(_, _, rules, zann) =>
      Typ.cursor_info(~steps=steps @ [1 + List.length(rules)], ctx, zann)
    | ApPaletteZ(_, _, _, zpsi) =>
      let (ty, ze) = ZNatMap.prj_z_v(zpsi.zsplice_map);
      ana_cursor_info(~steps, ctx, ze, ty);
    };
  }
  and ana_cursor_info =
      (~steps=[], ctx: Contexts.t, ze: ZExp.t, ty: HTyp.t): option(t) =>
    ana_cursor_info_zblock(~steps, ctx, ze, ty)
  and ana_cursor_info_zblock =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        (prefix, zline, suffix): ZExp.zblock,
        ty: HTyp.t,
      )
      : option(t) =>
    switch (Statics.Exp.syn_lines(ctx, prefix)) {
    | None => None
    | Some(ctx) =>
      switch (suffix) {
      | [] =>
        switch (zline) {
        | CursorL(_)
        | LetLineZP(_)
        | LetLineZA(_)
        | LetLineZE(_) => None
        | ExpLineZ(zopseq) =>
          ana_cursor_info_zopseq(
            ~steps=steps @ [List.length(prefix)],
            ctx,
            zopseq,
            ty,
          )
        }
      | [_, ..._] =>
        switch (
          syn_cursor_info_line(
            ~steps=steps @ [List.length(prefix)],
            ctx,
            zline,
          )
        ) {
        | None => None
        | Some(CursorNotOnDeferredVarPat(ci)) => Some(ci)
        | Some(CursorOnDeferredVarPat(deferred_ci, x)) =>
          let uses =
            UsageAnalysis.find_uses_block(
              ~offset=List.length(prefix) + 1,
              ~steps,
              x,
              suffix,
            );
          Some(uses |> deferred_ci);
        }
      }
    }
  and ana_cursor_info_zopseq =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        ZOpSeq(skel, zseq) as zopseq: ZExp.zopseq,
        ty: HTyp.t,
      )
      : option(t) => {
    let skel_tys = Statics.Exp.tuple_zip(skel, ty);
    switch (zseq) {
    | ZOperator((_, Comma), _) =>
      // cursor on tuple comma
      switch (skel_tys) {
      | Some(_) => Some(mk(Analyzed(ty), ctx))
      | None =>
        let expected_length = ty |> HTyp.get_prod_elements |> List.length;
        let got_length = skel |> UHExp.get_tuple_elements |> List.length;
        Some(mk(AnaWrongLength(expected_length, got_length, ty), ctx));
      }
    | _ =>
      // cursor in tuple element
      switch (skel_tys) {
      | None =>
        // wrong length, switch to syn
        let zopseq_not_in_hole =
          zopseq |> ZExp.set_err_status_zopseq(NotInHole);
        syn_cursor_info_zopseq(~steps, ctx, zopseq_not_in_hole);
      | Some(skel_tys) =>
        let (cursor_skel, cursor_skel_ty) =
          skel_tys
          |> List.find(((skel, _)) =>
               ZOpSeq.skel_contains_cursor(skel, zseq)
             );
        ana_cursor_info_skel(~steps, ctx, cursor_skel, zseq, cursor_skel_ty);
      }
    };
  }
  and ana_cursor_info_skel =
      // steps of whole opseq
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        skel: UHExp.skel,
        zseq: ZExp.zseq,
        ty: HTyp.t,
      )
      : option(t) => {
    let syn_go = skel => syn_cursor_info_skel(~steps, ctx, skel, zseq);
    let ana_go = (skel, ty) =>
      ana_cursor_info_skel(~steps, ctx, skel, zseq, ty);
    let seq = zseq |> ZExp.erase_zseq;
    if (ZOpSeq.skel_is_rooted_at_cursor(skel, zseq)) {
      // found cursor
      switch (zseq) {
      | ZOperand(zoperand, (prefix, _)) =>
        ana_cursor_info_zoperand(
          ~steps=steps @ [Seq.length_of_affix(prefix)],
          ctx,
          zoperand,
          ty,
        )
      | ZOperator(_) =>
        Statics.Exp.ana_skel(ctx, skel, seq, ty)
        |> OptUtil.map(_ => mk(Analyzed(ty), ctx))
      };
    } else {
      // recurse toward cursor
      switch (skel) {
      | Placeholder(_) => None
      | BinOp(InHole(_), _, _, _) => syn_go(skel)
      | BinOp(_, Comma, _, _) =>
        failwith(
          "Exp.ana_cursor_info_skel: expected commas too be handled at opseq level",
        )
      | BinOp(NotInHole, Cons, skel1, skel2) =>
        switch (HTyp.matched_list(ty)) {
        | None => None
        | Some(ty_elt) =>
          switch (ana_go(skel1, ty_elt)) {
          | Some(_) as result => result
          | None =>
            let ty_list = HTyp.List(ty_elt);
            ana_go(skel2, ty_list);
          }
        }
      | BinOp(
          _,
          Plus | Minus | Times | LessThan | GreaterThan | Equals | And | Or |
          Space,
          _,
          _,
        ) =>
        syn_go(skel)
      };
    };
  }
  and ana_cursor_info_zoperand =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        zoperand: ZExp.zoperand,
        ty: HTyp.t,
      )
      : option(t) =>
    switch (zoperand) {
    | CursorE(_, e) =>
      switch (e) {
      /* in hole */
      | Var(_, InVarHole(Keyword(k), _), _) =>
        Some(mk(AnaKeyword(ty, k), ctx))
      | Var(_, InVarHole(Free, _), _) => Some(mk(AnaFree(ty), ctx))
      | Var(InHole(TypeInconsistent, _), _, _)
      | NumLit(InHole(TypeInconsistent, _), _)
      | BoolLit(InHole(TypeInconsistent, _), _)
      | ListNil(InHole(TypeInconsistent, _))
      | Lam(InHole(TypeInconsistent, _), _, _, _)
      | Inj(InHole(TypeInconsistent, _), _, _)
      | Case(InHole(TypeInconsistent, _), _, _, _)
      | ApPalette(InHole(TypeInconsistent, _), _, _, _) =>
        let operand' =
          zoperand
          |> ZExp.erase_zoperand
          |> UHExp.set_err_status_operand(NotInHole);
        switch (Statics.Exp.syn_operand(ctx, operand')) {
        | None => None
        | Some(ty') => Some(mk(AnaTypeInconsistent(ty, ty'), ctx))
        };
      | Var(InHole(WrongLength, _), _, _)
      | NumLit(InHole(WrongLength, _), _)
      | BoolLit(InHole(WrongLength, _), _)
      | ListNil(InHole(WrongLength, _))
      | Lam(InHole(WrongLength, _), _, _, _)
      | Inj(InHole(WrongLength, _), _, _)
      | Case(InHole(WrongLength, _), _, _, _)
      | ApPalette(InHole(WrongLength, _), _, _, _) => None /* not in hole */
      | EmptyHole(_)
      | Var(NotInHole, NotInVarHole, _)
      | NumLit(NotInHole, _)
      | BoolLit(NotInHole, _)
      | ApPalette(NotInHole, _, _, _) =>
        switch (Statics.Exp.syn_operand(ctx, e)) {
        | None => None
        | Some(ty') => Some(mk(AnaSubsumed(ty, ty'), ctx))
        }
      | ListNil(NotInHole)
      | Inj(NotInHole, _, _)
      | Case(NotInHole, _, _, _) => Some(mk(Analyzed(ty), ctx))
      | Parenthesized(body) =>
        Statics.Exp.ana(ctx, body, ty)
        |> OptUtil.map(_ => mk(Analyzed(ty), ctx))
      | Lam(NotInHole, _, ann, _) =>
        switch (HTyp.matched_arrow(ty)) {
        | None => None
        | Some((ty1, ty2)) =>
          switch (ann) {
          | None => Some(mk(Analyzed(ty), ctx))
          | Some(ann) =>
            let ann_ty = ann |> UHTyp.expand;
            HTyp.consistent(ann_ty, ty1)
              ? Some(mk(AnaAnnotatedLambda(ty, Arrow(ann_ty, ty2)), ctx))
              : None;
          }
        }
      } /* zipper cases */
    | ParenthesizedZ(zbody) =>
      ana_cursor_info(~steps=steps @ [0], ctx, zbody, ty) /* zipper in hole */
    | LamZP(InHole(WrongLength, _), _, _, _)
    | LamZA(InHole(WrongLength, _), _, _, _)
    | LamZE(InHole(WrongLength, _), _, _, _)
    | InjZ(InHole(WrongLength, _), _, _)
    | CaseZE(InHole(WrongLength, _), _, _, _)
    | CaseZR(InHole(WrongLength, _), _, _, _)
    | CaseZA(InHole(WrongLength, _), _, _, _)
    | ApPaletteZ(InHole(WrongLength, _), _, _, _) => None
    | LamZP(InHole(TypeInconsistent, _), _, _, _)
    | LamZA(InHole(TypeInconsistent, _), _, _, _)
    | LamZE(InHole(TypeInconsistent, _), _, _, _)
    | InjZ(InHole(TypeInconsistent, _), _, _)
    | CaseZE(InHole(TypeInconsistent, _), _, _, _)
    | CaseZR(InHole(TypeInconsistent, _), _, _, _)
    | CaseZA(InHole(TypeInconsistent, _), _, _, _)
    | ApPaletteZ(InHole(TypeInconsistent, _), _, _, _) =>
      syn_cursor_info_zoperand(~steps, ctx, zoperand) /* zipper not in hole */
    | LamZP(NotInHole, zp, ann, body) =>
      switch (HTyp.matched_arrow(ty)) {
      | None => None
      | Some((ty1_given, _)) =>
        let ty1 =
          switch (ann) {
          | Some(uty1) => UHTyp.expand(uty1)
          | None => ty1_given
          };
        switch (Pat.ana_cursor_info(~steps=steps @ [0], ctx, zp, ty1)) {
        | None => None
        | Some(CursorNotOnDeferredVarPat(ci)) => Some(ci)
        | Some(CursorOnDeferredVarPat(deferred_ci, x)) =>
          let uses = UsageAnalysis.find_uses(~steps=steps @ [2], x, body);
          Some(uses |> deferred_ci);
        };
      }
    | LamZA(NotInHole, _, zann, _) =>
      Typ.cursor_info(~steps=steps @ [1], ctx, zann)
    | LamZE(NotInHole, p, ann, zbody) =>
      switch (HTyp.matched_arrow(ty)) {
      | None => None
      | Some((ty1_given, ty2)) =>
        let ty1 =
          switch (ann) {
          | Some(uty1) => UHTyp.expand(uty1)
          | None => ty1_given
          };
        switch (Statics.Pat.ana(ctx, p, ty1)) {
        | None => None
        | Some(ctx) => ana_cursor_info(~steps=steps @ [2], ctx, zbody, ty2)
        };
      }
    | InjZ(NotInHole, position, zbody) =>
      switch (HTyp.matched_sum(ty)) {
      | None => None
      | Some((ty1, ty2)) =>
        ana_cursor_info(
          ~steps=steps @ [0],
          ctx,
          zbody,
          InjSide.pick(position, ty1, ty2),
        )
      }
    | CaseZE(NotInHole, zscrut, _, _) =>
      syn_cursor_info(~steps=steps @ [0], ctx, zscrut)
    | CaseZR(NotInHole, scrut, (prefix, zrule, _), _) =>
      switch (Statics.Exp.syn(ctx, scrut)) {
      | None => None
      | Some(ty1) =>
        ana_cursor_info_rule(
          ~steps=steps @ [1 + List.length(prefix)],
          ctx,
          zrule,
          ty1,
          ty,
        )
      }
    | CaseZA(NotInHole, _, rules, zann) =>
      Typ.cursor_info(~steps=steps @ [1 + List.length(rules)], ctx, zann)
    | ApPaletteZ(NotInHole, _, _, _) =>
      syn_cursor_info_zoperand(~steps, ctx, zoperand)
    }
  and ana_cursor_info_rule =
      (
        ~steps: CursorPath.steps,
        ctx: Contexts.t,
        zrule: ZExp.zrule,
        pat_ty: HTyp.t,
        clause_ty: HTyp.t,
      )
      : option(t) =>
    switch (zrule) {
    | CursorR(_) => Some(mk(OnRule, ctx))
    | RuleZP(zp, clause) =>
      switch (Pat.ana_cursor_info(~steps=steps @ [0], ctx, zp, pat_ty)) {
      | None => None
      | Some(CursorNotOnDeferredVarPat(ci)) => Some(ci)
      | Some(CursorOnDeferredVarPat(deferred_ci, x)) =>
        let uses = UsageAnalysis.find_uses(~steps=steps @ [1], x, clause);
        Some(uses |> deferred_ci);
      }
    | RuleZE(p, zclause) =>
      switch (Statics.Pat.ana(ctx, p, pat_ty)) {
      | None => None
      | Some(ctx) =>
        ana_cursor_info(~steps=steps @ [1], ctx, zclause, clause_ty)
      }
    };
};
