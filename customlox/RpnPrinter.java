package com.craftinginterpreters.lox;

class RpnPrinter implements Expr.Visitor<String>{
    String print(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        throw new RuntimeException("RPN not supported for assignment.");
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return expr.left.accept(this) + " " + expr.right.accept(this) + " " + expr.operator.lexeme;
    }

    @Override
    public String visitCallExpr(Expr.Call expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

    @Override
    public String visitGetExpr(Expr.Get expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return expr.expression.accept(this);
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        return expr.value.toString();
    }

    @Override
    public String visitLogicalExpr(Expr.Logical expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

    @Override
    public String visitSetExpr(Expr.Set expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

    @Override
    public String visitSuperExpr(Expr.Super expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

    @Override
    public String visitThisExpr(Expr.This expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return expr.right.accept(this) + " " + expr.operator.lexeme;
    }

    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        throw new RuntimeException("RPN not supported for assignment");
    }

}