/*!
 * @file Order.h
 * @author Domagoj Markota <domagoj.markota@gmail.com>
 */


#ifndef ORDER_H
#define ORDER_H


#include <string>


class Order {
public:
    static constexpr unsigned STR_MAX_SIZE = 20u;

private:
    unsigned id;
    char operation;
    std::string symbol;
    std::string side;
    unsigned volume;
    double price;
    std::string timestamp;

protected:
    Order() = default;

public:
    explicit Order(
        const unsigned & id,
        const char & operation,
        const std::string & symbol,
        const std::string & side,
        const unsigned & volume,
        const double & price,
        const std::string & timestamp
    );

    virtual ~Order() = default;

    Order & operator=(Order & order);

public:
    const auto & getId() const;
    const auto & getOperation() const;
    const auto & getSymbol() const;
    const auto & getSide() const;
    const auto & getVolume() const;
    const auto & getPrice() const;
    const auto & getTimestamp() const;

};



Order::Order(
    const unsigned & id,
    const char & operation,
    const std::string & symbol,
    const std::string & side,
    const unsigned & volume,
    const double & price,
    const std::string & timestamp
):
    id(id),
    operation(operation),
    symbol(symbol),
    side(side),
    volume(volume),
    price(price),
    timestamp(timestamp)
{
}



Order & Order::operator=(Order & order) {
    this->id = order.id;
    this->operation = order.operation;
    this->symbol = order.symbol;
    this->side = order.side;
    this->volume = order.volume;
    this->price = order.price;
    this->timestamp = order.timestamp;

    return *this;
}



const auto & Order::getId() const {
    return this->id;
}



const auto & Order::getOperation() const {
    return this->operation;
}



const auto & Order::getSymbol() const {
    return this->symbol;
}



const auto & Order::getSide() const {
    return this->side;
}



const auto & Order::getVolume() const {
    return this->volume;
}



const auto & Order::getPrice() const {
    return this->price;
}



const auto & Order::getTimestamp() const {
    return this->timestamp;
}



#endif // ORDER_H