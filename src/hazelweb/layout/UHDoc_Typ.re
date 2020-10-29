let inline_padding_of_operator:
  UHTyp.operator => (UHDoc_common.t, UHDoc_common.t) =
  fun
  | Prod => (UHDoc_common.empty_, UHDoc_common.space_)
  | Arrow
  | Sum
  | Space => (UHDoc_common.space_, UHDoc_common.space_);

let mk_EmptyHole: string => UHDoc_common.t =
  UHDoc_common.mk_EmptyHole(~sort=Typ);
let mk_Parenthesized: UHDoc_common.formatted_child => UHDoc_common.t =
  UHDoc_common.mk_Parenthesized(~sort=Typ);
let mk_NTuple:
  (
    ~mk_operand: (~enforce_inline: bool, 'a) => UHDoc_common.t,
    ~mk_operator: UHTyp.operator => UHDoc_common.t,
    ~enforce_inline: bool,
    OpSeq.t('a, UHTyp.operator)
  ) =>
  UHDoc_common.t =
  UHDoc_common.mk_NTuple(
    ~sort=Typ,
    ~get_tuple_elements=UHTyp.get_prod_elements,
    ~inline_padding_of_operator,
  );

let rec mk =
  lazy(
    UHDoc_common.memoize(
      (~memoize: bool, ~enforce_inline: bool, uty: UHTyp.t) =>
      (Lazy.force(mk_opseq, ~memoize, ~enforce_inline, uty): UHDoc_common.t)
    )
  )
and mk_opseq =
  lazy(
    UHDoc_common.memoize(
      (~memoize: bool, ~enforce_inline: bool, opseq: UHTyp.opseq) =>
      (
        mk_NTuple(
          ~mk_operand=Lazy.force(mk_operand, ~memoize),
          ~mk_operator,
          ~enforce_inline,
          opseq,
        ): UHDoc_common.t
      )
    )
  )
and mk_operator = (op: UHTyp.operator): UHDoc_common.t =>
  UHDoc_common.mk_op(Operators_Typ.to_string(op))
and mk_operand =
  lazy(
    UHDoc_common.memoize(
      (~memoize: bool, ~enforce_inline: bool, operand: UHTyp.operand) =>
      (
        switch (operand) {
        | Hole => mk_EmptyHole("?")
        | Unit => UHDoc_common.mk_Unit()
        | Int => UHDoc_common.mk_Int()
        | Float => UHDoc_common.mk_Float()
        | Bool => UHDoc_common.mk_Bool()
        | Parenthesized(body) =>
          let body = mk_child(~memoize, ~enforce_inline, ~child_step=0, body);
          mk_Parenthesized(body);
        | List(body) =>
          let body = mk_child(~memoize, ~enforce_inline, ~child_step=0, body);
          UHDoc_common.mk_List(body);
        // ECD TODO: Show the label error in this case
        | Label(_, label) => UHDoc_common.mk_Label(~sort=Typ, label)
        }: UHDoc_common.t
      )
    )
  )
and mk_child =
    (~memoize: bool, ~enforce_inline: bool, ~child_step: int, uty: UHTyp.t)
    : UHDoc_common.formatted_child => {
  let formattable = (~enforce_inline: bool) =>
    Lazy.force(mk, ~memoize, ~enforce_inline, uty)
    |> UHDoc_common.annot_Step(child_step);
  enforce_inline
    ? EnforcedInline(formattable(~enforce_inline=true))
    : Unformatted(formattable);
};
