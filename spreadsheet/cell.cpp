#include "cell.h"

using namespace std::literals;

// class Cell

CellInterface::Value Cell::GetValue() const {
    if (value_.has_value()){
        return value_.value();
    }
    const CellInterface::Value value = impl_ -> GetValue();
    if (std::holds_alternative<double>(value)){
        value_ = std::get<double>(value);
    }
    return value;
}

Position& Cell::GetPos(){
    return pos_;
}

const Position& Cell::GetPos() const {
    return pos_;
}

void Cell::SetPos(Position pos){
    pos_ = pos;
}

std::string Cell::GetText() const {
    return impl_ -> GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_ -> GetReferencedCells();
}

const std::unique_ptr<Impl>& Cell::GetImpl() const {
    return impl_;
}

std::unique_ptr<Impl>& Cell::GetImpl(){
    return impl_;
}

void Cell::Set(std::unique_ptr<Impl> impl){
    impl_ = std::move(impl);
}

void Cell::Clear(){
    ResetCache();
    impl_.reset();
}

void Cell::ResetCache(){
    value_.reset();
}

// class Impl;

CellInterface::Value EmptyImpl::GetValue() const {
    return {};
}

std::string EmptyImpl::GetText() const {
    return {};
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    return {};
}

CellInterface::Value TextImpl::GetValue() const {
    if (text_[0] == ESCAPE_SIGN){
        return text_.substr(1);
    }
    return text_;
}

std::string TextImpl::GetText() const {
    return text_;
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    return {};
}

CellInterface::Value FormulaImpl::GetValue() const {
    const auto value = ast_ -> Evaluate(sheet_);
    if (std::holds_alternative<FormulaError>(value)){
        return std::get<FormulaError>(value);
    }
    return std::get<double>(value);
}

std::string FormulaImpl::GetText() const {
    return FORMULA_SIGN + ast_ -> GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return ast_ -> GetReferencedCells();
}
