#pragma once

#include "common.h"
#include "formula.h"

#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>


struct PositionHash{
    size_t operator()(const Position& pos) const noexcept {
        return pos.row + pos.col * 37;
    }
};

class Impl {
public:
    virtual ~Impl() = default;

    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class EmptyImpl : public Impl {
public:
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
};

class TextImpl: public Impl {
private:
    std::string text_;
public:
    TextImpl(std::string text)
        : text_(std::move(text))
    {}
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
};


class FormulaImpl: public Impl{
private:
    std::unique_ptr<FormulaInterface> ast_;
    const SheetInterface& sheet_;

    std::vector<Position> refferenced_cells_;
public:
    FormulaImpl(std::string expression,  const SheetInterface& sheet)
        : ast_(ParseFormula(expression))
        , sheet_(sheet)
        , refferenced_cells_(ast_ -> GetReferencedCells()){}

    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
};


class  Cell: public CellInterface {
private:

    mutable std::optional<double> value_; //Кешируемая значение для формульной ячейки

    Position current_pos_;

    std::unique_ptr<Impl> impl_ = std::make_unique<EmptyImpl>(); // Уникальная ссылка

    const SheetInterface& sheet_;

    void CheckCircularDependency(Position current_pos, const std::vector<Position>& reff_cells);

public:
    explicit Cell(const SheetInterface& sheet)
        : sheet_(sheet)
    {}
    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void ResetCache();

    const std::unique_ptr<Impl>& GetImpl() const;
    std::unique_ptr<Impl>& GetImpl();

    Position& GetPos();

    const Position& GetPos() const;

    void SetPos(Position pos);

    void Clear();

    void Set(std::string text);

    operator bool() const {
        return impl_ != nullptr;
    }
};
