package lox;

class LoopControl extends RuntimeException {
    final Token keyword;

    LoopControl(Token keyword) {
        super(null, null, false, false);
        this.keyword = keyword;
    }
}
