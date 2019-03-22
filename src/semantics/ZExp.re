open SemanticsCommon;
open HazelUtil;

[@deriving show({with_path: false})]
type cursor_side = SemanticsCommon.cursor_side;

type opseq_surround = OperatorSeq.opseq_surround(UHExp.t, UHExp.op);
type opseq_prefix = OperatorSeq.opseq_prefix(UHExp.t, UHExp.op);
type opseq_suffix = OperatorSeq.opseq_suffix(UHExp.t, UHExp.op);

type zblock =
  /*
   TODO add block level cursor
   | CursorB(cursor_side, UHExp.block)
   */
  | DeeperB(zblock')
and zblock' =
  | BlockZL(zlines, UHExp.t)
  | BlockZE(UHExp.lines, t)
and zlines = ZList.t(zline, UHExp.line)
and zline =
  | CursorL(cursor_side, UHExp.line)
  | DeeperL(zline')
and zline' =
  | ExpLineZ(t)
  | LetLineZP(ZPat.t, option(UHTyp.t), UHExp.block)
  | LetLineZA(UHPat.t, ZTyp.t, UHExp.block)
  | LetLineZE(UHPat.t, option(UHTyp.t), zblock)
and t =
  | CursorE(cursor_side, UHExp.t)
  | ParenthesizedZ(zblock)
  | OpSeqZ(UHExp.skel_t, t, OperatorSeq.opseq_surround(UHExp.t, UHExp.op))
  | DeeperE(err_status, t')
/* | CursorPalette : PaletteName.t -> PaletteSerializedModel.t -> hole_ref -> t -> t */
and t' =
  | LamZP(ZPat.t, option(UHTyp.t), UHExp.block)
  | LamZA(UHPat.t, ZTyp.t, UHExp.block)
  | LamZE(UHPat.t, option(UHTyp.t), zblock)
  | InjZ(inj_side, zblock)
  | CaseZE(zblock, list(UHExp.rule), option(UHTyp.t))
  | CaseZR(UHExp.block, zrules, option(UHTyp.t))
  | CaseZA(UHExp.block, list(UHExp.rule), ZTyp.t)
  | ApPaletteZ(
      PaletteName.t,
      SerializedModel.t,
      ZSpliceInfo.t(UHExp.block, zblock),
    )
and zrules = ZList.t(zrule, UHExp.rule)
and zrule =
  | RuleZP(ZPat.t, UHExp.block)
  | RuleZE(UHPat.t, zblock);

let bidelimit = (ze: t): t =>
  switch (ze) {
  | CursorE(cursor_side, e) => CursorE(cursor_side, UHExp.bidelimit(e))
  | ParenthesizedZ(_)
  | DeeperE(_, InjZ(_, _))
  | DeeperE(_, ApPaletteZ(_, _, _)) =>
    /* | Deeper _ (ListLitZ _) */
    ze
  | OpSeqZ(_, _, _)
  | DeeperE(_, CaseZE(_, _, _))
  | DeeperE(_, CaseZR(_, _, _))
  | DeeperE(_, CaseZA(_, _, _))
  | DeeperE(_, LamZP(_, _, _))
  | DeeperE(_, LamZA(_, _, _))
  | DeeperE(_, LamZE(_, _, _)) => ParenthesizedZ(DeeperB(BlockZE([], ze)))
  };

let rec set_err_status = (err: err_status, ze: t): t =>
  switch (ze) {
  | CursorE(cursor_side, e) =>
    let e = UHExp.set_err_status(err, e);
    CursorE(cursor_side, e);
  | OpSeqZ(BinOp(_, op, skel1, skel2), ze0, surround) =>
    OpSeqZ(BinOp(err, op, skel1, skel2), ze0, surround)
  | OpSeqZ(Placeholder(_), _, _) => ze /* should never happen */
  | DeeperE(_, ze') => DeeperE(err, ze')
  | ParenthesizedZ(DeeperB(BlockZL(zlines, e))) =>
    ParenthesizedZ(DeeperB(BlockZL(zlines, UHExp.set_err_status(err, e))))
  | ParenthesizedZ(DeeperB(BlockZE(lines, ze))) =>
    ParenthesizedZ(DeeperB(BlockZE(lines, set_err_status(err, ze))))
  };

let rec make_inconsistent = (u_gen: MetaVarGen.t, ze: t): (t, MetaVarGen.t) =>
  switch (ze) {
  | CursorE(cursor_side, e) =>
    let (e', u_gen) = UHExp.make_inconsistent(u_gen, e);
    (CursorE(cursor_side, e'), u_gen);
  | DeeperE(NotInHole, ze')
  | DeeperE(InHole(WrongLength, _), ze') =>
    let (u, u_gen) = MetaVarGen.next(u_gen);
    let ze' = set_err_status(InHole(TypeInconsistent, u), ze);
    (ze', u_gen);
  | DeeperE(InHole(TypeInconsistent, _), _) => (ze, u_gen)
  | ParenthesizedZ(DeeperB(BlockZL(zlines, e))) =>
    let (e, u_gen) = UHExp.make_inconsistent(u_gen, e);
    (ParenthesizedZ(DeeperB(BlockZL(zlines, e))), u_gen);
  | ParenthesizedZ(DeeperB(BlockZE(lines, ze))) =>
    let (ze, u_gen) = make_inconsistent(u_gen, ze);
    (ParenthesizedZ(DeeperB(BlockZE(lines, ze))), u_gen);
  | OpSeqZ(BinOp(NotInHole, _, _, _), _, _)
  | OpSeqZ(BinOp(InHole(WrongLength, _), _, _, _), _, _) =>
    let (u, u_gen) = MetaVarGen.next(u_gen);
    let ze = set_err_status(InHole(TypeInconsistent, u), ze);
    (ze, u_gen);
  | OpSeqZ(BinOp(InHole(TypeInconsistent, _), _, _, _), _, _) => (
      ze,
      u_gen,
    )
  | OpSeqZ(Placeholder(_), _, _) => (ze, u_gen) /* should never happen */
  };

let new_EmptyHole = (u_gen: MetaVarGen.t) => {
  let (e, u_gen) = UHExp.new_EmptyHole(u_gen);
  (CursorE(Before, e), u_gen);
};

/*
 let rec cursor_on_outer_expr = (ze: t): option((UHExp.t, cursor_side)) =>
   switch (ze) {
   | CursorE(side, e) => Some((UHExp.drop_outer_parentheses(e), side))
   | ParenthesizedZ(ze') => cursor_on_outer_expr(ze')
   | DeeperE(_, _) => None
   };
 */

let empty_zrule = (u_gen: MetaVarGen.t): (zrule, MetaVarGen.t) => {
  let (zp, u_gen) = ZPat.new_EmptyHole(u_gen);
  let (e, u_gen) = UHExp.new_EmptyHole(u_gen);
  let block = UHExp.Block([], e);
  let zrule = RuleZP(zp, block);
  (zrule, u_gen);
};

let rec erase_block = (zblock: zblock): UHExp.block =>
  switch (zblock) {
  | DeeperB(BlockZL(zlines, e)) => Block(erase_lines(zlines), e)
  | DeeperB(BlockZE(lines, ze)) => Block(lines, erase(ze))
  }
and erase_lines = (zlis: zlines): UHExp.lines =>
  ZList.erase(zlis, erase_line)
and erase_line = (zli: zline): UHExp.line =>
  switch (zli) {
  | CursorL(_, li) => li
  | DeeperL(zli') => erase_line'(zli')
  }
and erase_line' = (zli': zline'): UHExp.line =>
  switch (zli') {
  | ExpLineZ(ze) => ExpLine(erase(ze))
  | LetLineZP(zp, ann, block) => LetLine(ZPat.erase(zp), ann, block)
  | LetLineZA(p, zann, block) => LetLine(p, Some(ZTyp.erase(zann)), block)
  | LetLineZE(p, ann, zblock) => LetLine(p, ann, erase_block(zblock))
  }
and erase = (ze: t): UHExp.t =>
  switch (ze) {
  | CursorE(_, e) => e
  | DeeperE(err_state, ze') =>
    let e' = erase'(ze');
    Tm(err_state, e');
  | ParenthesizedZ(zblock) => UHExp.Parenthesized(erase_block(zblock))
  | OpSeqZ(skel, ze', surround) =>
    let e = erase(ze');
    OpSeq(skel, OperatorSeq.opseq_of_exp_and_surround(e, surround));
  }
and erase' = (ze: t'): UHExp.t' =>
  switch (ze) {
  | LamZP(zp, ann, block) => Lam(ZPat.erase(zp), ann, block)
  | LamZA(p, zann, block) => Lam(p, Some(ZTyp.erase(zann)), block)
  | LamZE(p, ann, zblock) => Lam(p, ann, erase_block(zblock))
  | InjZ(side, zblock) => Inj(side, erase_block(zblock))
  | CaseZE(zblock, rules, ann) => Case(erase_block(zblock), rules, ann)
  | CaseZR(block, zrules, ann) =>
    Case(block, ZList.erase(zrules, erase_rule), ann)
  | CaseZA(e1, rules, zann) =>
    UHExp.Case(e1, rules, Some(ZTyp.erase(zann)))
  | ApPaletteZ(palette_name, serialized_model, zpsi) =>
    let psi = ZSpliceInfo.erase(zpsi, ((ty, z)) => (ty, erase_block(z)));
    ApPalette(palette_name, serialized_model, psi);
  }
and erase_rule = (zr: zrule): UHExp.rule =>
  switch (zr) {
  | RuleZP(zp, block) => Rule(ZPat.erase(zp), block)
  | RuleZE(p, zblock) => Rule(p, erase_block(zblock))
  };

let rec cursor_at_start = (ze: t): bool =>
  switch (ze) {
  | CursorE(Before, _) => true
  | CursorE(_, _) => false
  | ParenthesizedZ(_) => false
  | DeeperE(_, LineItemZL(zli, _)) => cursor_at_start_line_item(zli)
  | DeeperE(_, LineItemZE(_, _)) => false
  | DeeperE(_, LamZP(_, _, _))
  | DeeperE(_, LamZA(_, _, _))
  | DeeperE(_, LamZE(_, _, _)) => false
  | DeeperE(_, InjZ(_, _)) => false
  | DeeperE(_, CaseZE(_, _, _))
  | DeeperE(_, CaseZR(_, _, _))
  | DeeperE(_, CaseZA(_, _, _)) => false
  | DeeperE(_, OpSeqZ(_, ze1, EmptyPrefix(_))) => cursor_at_start(ze1)
  | DeeperE(_, OpSeqZ(_, _, _)) => false
  | DeeperE(_, ApPaletteZ(_, _, _)) => false
  }
and cursor_at_start_line_item = (zli: zline_item): bool =>
  switch (zli) {
  | CursorL(_, EmptyLine) => true
  | CursorL(Before, _) => true
  | CursorL(_, _) => false
  | DeeperL(zli') => cursor_at_start_line_item'(zli')
  }
and cursor_at_start_line_item' = (zli': zline_item'): bool =>
  switch (zli') {
  | ExpLineZ(ze) => cursor_at_start(ze)
  | LetLineZP(_, _, _)
  | LetLineZA(_, _, _)
  | LetLineZE(_, _, _) => false
  };

let rec cursor_at_end = (ze: t): bool =>
  switch (ze) {
  | CursorE(After, _) => true
  | CursorE(_, _) => false
  | ParenthesizedZ(_) => false
  | DeeperE(_, LineItemZL(_, _)) => false
  | DeeperE(_, LineItemZE(_, ze1)) => cursor_at_end(ze1)
  | DeeperE(_, LamZP(_, _, _))
  | DeeperE(_, LamZA(_, _, _)) => false
  | DeeperE(_, LamZE(_, _, ze1)) => cursor_at_end(ze1)
  | DeeperE(_, InjZ(_, _)) => false
  | DeeperE(_, CaseZE(_, _, _))
  | DeeperE(_, CaseZR(_, _, _)) => false
  | DeeperE(_, CaseZA(_, _, zann)) => ZTyp.cursor_at_end(zann)
  | DeeperE(_, OpSeqZ(_, ze1, EmptySuffix(_))) => cursor_at_end(ze1)
  | DeeperE(_, OpSeqZ(_, _, _)) => false
  | DeeperE(_, ApPaletteZ(_, _, _)) => false
  };

let cursor_at_end_line_item' = (zli': zline_item'): bool =>
  switch (zli') {
  | ExpLineZ(ze) => cursor_at_end(ze)
  | LetLineZP(_, _, _)
  | LetLineZA(_, _, _) => false
  | LetLineZE(_, _, ze) => cursor_at_end(ze)
  };
let cursor_at_end_line_item = (zli: zline_item): bool =>
  switch (zli) {
  | CursorL(_, EmptyLine) => true
  | CursorL(After, _) => true
  | CursorL(_, _) => false
  | DeeperL(zli') => cursor_at_end_line_item'(zli')
  };

let rec place_Before = (e: UHExp.t): t =>
  switch (e) {
  | Tm(err_status, LineItem(EmptyLine, e1)) =>
    DeeperE(err_status, LineItemZL(CursorL(Before, EmptyLine), e1))
  | Tm(err_status, LineItem(ExpLine(e1), e2)) =>
    let ze1 = place_Before(e1);
    DeeperE(err_status, LineItemZL(DeeperL(ExpLineZ(ze1)), e2));
  | Tm(err_status, LineItem(LetLine(_, _, _), _)) =>
    /* TODO this selects the entire block, perhaps should consider enabling selecting single line items */
    CursorE(Before, e)
  | Tm(err_status, OpSeq(skel, opseq)) =>
    let (e1, suffix) = OperatorSeq.split0(opseq);
    let ze1 = place_Before(e1);
    let surround = OperatorSeq.EmptyPrefix(suffix);
    DeeperE(err_status, OpSeqZ(skel, ze1, surround));
  | Tm(_, Var(_, _))
  | Tm(_, Lam(_, _, _))
  | Tm(_, NumLit(_))
  | Tm(_, BoolLit(_))
  | Tm(_, Inj(_, _))
  | Tm(_, Case(_, _, _))
  | Tm(_, ListNil)
  | Tm(_, EmptyHole(_))
  | Tm(_, ApPalette(_, _, _))
  | Parenthesized(_) => CursorE(Before, e)
  };

let rec place_After = (e: UHExp.t): t =>
  switch (e) {
  | Tm(err_status, LineItem(li, e2)) =>
    let ze2 = place_After(e2);
    DeeperE(err_status, LineItemZE(li, ze2));
  | Tm(err_status, OpSeq(skel, opseq)) =>
    let (en, prefix) = OperatorSeq.split_tail(opseq);
    let zen = place_After(en);
    let surround = OperatorSeq.EmptySuffix(prefix);
    DeeperE(err_status, OpSeqZ(skel, zen, surround));
  | Tm(_, Case(e, rules, Some(ty))) =>
    DeeperE(NotInHole, CaseZA(e, rules, ZTyp.place_After(ty)))
  | Tm(_, Case(_, _, None))
  | Tm(_, Var(_, _))
  | Tm(_, Lam(_, _, _))
  | Tm(_, NumLit(_))
  | Tm(_, BoolLit(_))
  | Tm(_, Inj(_, _))
  | Tm(_, ListNil)
  | Tm(_, EmptyHole(_))
  | Tm(_, ApPalette(_, _, _))
  | Parenthesized(_) => CursorE(After, e)
  };

let rec place_After_line_item = (li: UHExp.line_item): zline_item =>
  switch (li) {
  | EmptyLine => CursorL(After, li)
  | ExpLine(e) => DeeperL(ExpLineZ(place_After(e)))
  | LetLine(p, ann, e) => DeeperL(LetLineZE(p, ann, place_After(e)))
  };

let prune_single_hole_line = (zli: zline_item): zline_item =>
  switch (zli) {
  | CursorL(side, li) => CursorL(side, UHExp.prune_single_hole_line(li))
  | DeeperL(ExpLineZ(CursorE(_, Tm(_, EmptyHole(_))))) =>
    CursorL(Before, EmptyLine)
  | DeeperL(ExpLineZ(_))
  | DeeperL(LetLineZP(_, _, _))
  | DeeperL(LetLineZA(_, _, _))
  | DeeperL(LetLineZE(_, _, _)) => zli
  };

let prepend_line = (~err_status=?, li: UHExp.line_item, ze: t): t =>
  DeeperE(default_nih(err_status), LineItemZE(li, ze));

let prepend_zline = (~err_status=?, zli: zline_item, e: UHExp.t): t =>
  DeeperE(default_nih(err_status), LineItemZL(zli, e));

let prune_and_prepend_line = (~err_status=?, li: UHExp.line_item, ze: t): t =>
  prepend_line(
    ~err_status=default_nih(err_status),
    UHExp.prune_single_hole_line(li),
    ze,
  );

let rec prune_and_prepend_lines = (e1: UHExp.t, ze2: t): t =>
  switch (e1) {
  | Tm(_, LineItem(li, e3)) =>
    prune_and_prepend_line(li, prune_and_prepend_lines(e3, ze2))
  | Tm(_, _)
  | Parenthesized(_) => prune_and_prepend_line(ExpLine(e1), ze2)
  };

let prune_and_prepend_zline = (~err_status=?, zli: zline_item, e: UHExp.t): t =>
  prepend_zline(
    ~err_status=default_nih(err_status),
    prune_single_hole_line(zli),
    e,
  );
