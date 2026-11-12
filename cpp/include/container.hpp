#pragma once
#include <string>
#include "serializable.hpp"

class Array;

class Container : public Serializable {
public:
    [[nodiscard]] virtual int length() const = 0;
    [[nodiscard]] bool empty() const { return length() == 0; }
    virtual void clear() = 0;
    [[nodiscard]] virtual std::string type_name() const = 0;
};

class Sequence : public Container {
public:
    [[nodiscard]] virtual Array values() const = 0;

    void save_binary(std::ostream& out) const override;
    void load_binary(std::istream& in) override;
    void save_text(std::ostream& out) const override;
    void load_text(std::istream& in) override;

protected:
    virtual void restore(const std::string& value) = 0;
};
