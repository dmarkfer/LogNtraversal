/*!
 * @file Storage.h
 * @author Domagoj Markota <domagoj.markota@gmail.com>
 */


#ifndef STORAGE_H
#define STORAGE_H


#include "Order.h"

#include <memory>
#include <cstdio>
#include <string>
#include <map>
#include <utility>
#include <set>
#include <vector>
#include <iostream>


class Storage final {
private:
    // Full binary tree, Memory Winner/Tournament, dynamic expansion
    struct PriceTournamentStruct {
        struct Node {
            int orderId;
            double price;
            int volume;
            std::shared_ptr<Node> l;
            std::shared_ptr<Node> r;

            Node(): orderId(-1), price(-1.), volume(-1) { }

            Node(
                const int & orderId,
                const double & price,
                const int & volume
            ): orderId(orderId), price(price), volume(volume) { }

            Node(
                const int & orderId,
                const double & price,
                const int & volume,
                const std::shared_ptr<Node> & l,
                const std::shared_ptr<Node> & r
            ): orderId(orderId), price(price), volume(volume), l(l), r(r) { }

        };

        unsigned treeDepth;
        unsigned leafsN;

        std::map<std::string, std::shared_ptr<Node>> root;
        std::map<std::string, std::shared_ptr<Node>>::iterator lastRoot;

        PriceTournamentStruct(): treeDepth(0u), leafsN(0u) { }

        void expandTree(
            const unsigned && currentDepth,
            const std::shared_ptr<Node> & node,
            const int & orderId,
            const double & price,
            const int & volume
        ) {
            if(currentDepth == this->treeDepth) {
                return;
            }

            node->l = std::make_shared<Node>(orderId, price, volume);
            node->r = std::make_shared<Node>();

            this->expandTree(currentDepth + 1u, node->l, orderId, price, volume);
            this->expandTree(currentDepth + 1u, node->r, -1, -1., -1);
        }

        const std::shared_ptr<Node> setValue(
            const std::shared_ptr<Node> & node,
            const unsigned & lo,
            const unsigned & hi,
            const int & orderId,
            const unsigned & ordinalNumber,
            const double & price,
            const int & volume
        ) {
            if(ordinalNumber >= lo  &&  ordinalNumber < hi) { // int doesn't affect, since non-negative number is always passed
                if(lo + 1u == hi) {
                    return std::make_shared<Node>(
                        orderId,
                        price,
                        volume
                    );
                } else {
                    unsigned mid = (lo + hi) / 2;
                    
                    auto left = this->setValue(node->l, lo, mid, orderId, ordinalNumber, price, volume);
                    auto right = this->setValue(node->r, mid, hi, orderId, ordinalNumber, price, volume);

                    auto greater = left;
                    if(right->price > greater->price) {
                        greater = right;
                    } else if(right->price == greater->price) {
                        if(right->volume > greater->volume) {
                            greater = right;
                        }
                    }

                    return std::make_shared<Node>(
                        greater->orderId,
                        greater->price,
                        greater->volume,
                        left,
                        right
                    );
                }
            } else {
                return node;
            }
        }

        const std::shared_ptr<Node> emptyLeaf(
            const std::shared_ptr<Node> & node,
            const unsigned & lo,
            const unsigned & hi,
            const unsigned & ordinalNumber
        ) {
            if(ordinalNumber >= lo  &&  ordinalNumber < hi) {
                if(lo + 1u == hi) {
                    return std::make_shared<Node>();
                } else {
                    unsigned mid = (lo + hi) / 2;
                    
                    auto left = this->emptyLeaf(node->l, lo, mid, ordinalNumber);
                    auto right = this->emptyLeaf(node->r, mid, hi, ordinalNumber);
                    
                    auto greater = left;
                    if(right->price > greater->price) {
                        greater = right;
                    } else if(right->price == greater->price) {
                        if(right->volume > greater->volume) {
                            greater = right;
                        }
                    }

                    return std::make_shared<Node>(
                        greater->orderId,
                        greater->price,
                        greater->volume,
                        left,
                        right
                    );
                }
            } else {
                return node;
            }
        }

        void insert(
            const int & orderId,
            const unsigned & ordinalNumber,
            const double & price,
            const int & volume,
            const std::string & timestamp
        ) {
            if(ordinalNumber >= this->leafsN) {
                auto secondLastRoot = this->lastRoot;


                const auto rootIter = this->root.find(timestamp);

                if(rootIter == this->root.end()) {
                    this->lastRoot = this->root.insert({
                        timestamp,
                        std::make_shared<Node>(orderId, price, volume)
                    }).first;
                } else {
                    rootIter->second = std::make_shared<Node>(orderId, price, volume);
                }

                if(! this->leafsN) {
                    this->leafsN = this->treeDepth = 1;
                } else {
                    ++this->treeDepth;
                    this->leafsN <<= 1;

                    if(secondLastRoot->second->price > price) {
                        this->lastRoot->second->orderId = secondLastRoot->second->orderId;
                        this->lastRoot->second->price = secondLastRoot->second->price;
                        this->lastRoot->second->volume = secondLastRoot->second->volume;
                    }

                    this->lastRoot->second->l = secondLastRoot->second;
                    this->lastRoot->second->r = std::make_shared<Node>(
                        orderId,
                        price,
                        volume
                    );

                    this->expandTree(2u, this->lastRoot->second->r, orderId, price, volume);
                }
            } else {
                const auto rootIter = this->root.find(timestamp);

                if(rootIter == this->root.end()) {
                    this->lastRoot = this->root.insert({
                        timestamp,
                        this->setValue(this->lastRoot->second, 0u, this->leafsN, orderId, ordinalNumber, price, volume)
                    }).first;
                } else {
                    rootIter->second = this->setValue(this->lastRoot->second, 0u, this->leafsN, orderId, ordinalNumber, price, volume);
                }
            }
        }

        void amend(
            const int & orderId,
            const unsigned & ordinalNumber,
            const double & price,
            const int & volume,
            const std::string & timestamp
        ) {
            const auto rootIter = this->root.find(timestamp);

            if(rootIter == this->root.end()) {
                this->lastRoot = this->root.insert({
                    timestamp,
                    this->setValue(this->lastRoot->second, 0u, this->leafsN, orderId, ordinalNumber, price, volume)
                }).first;
            } else {
                rootIter->second = this->setValue(this->lastRoot->second, 0u, this->leafsN, orderId, ordinalNumber, price, volume);
            }
        }

        void cancel(
            const unsigned & ordinalNumber,
            const std::string & timestamp
        ) {
            const auto rootIter = this->root.find(timestamp);

            if(rootIter == this->root.end()) {
                this->lastRoot = this->root.insert({
                    timestamp,
                    this->emptyLeaf(this->lastRoot->second, 0u, this->leafsN, ordinalNumber)
                }).first;
            } else {
                rootIter->second = this->emptyLeaf(this->lastRoot->second, 0u, this->leafsN, ordinalNumber);
            }
        }
        
        const std::pair<int, std::pair<double, int>> getBest(const std::string & timestamp) {
            auto rootIter = this->root.upper_bound(timestamp);

            if(rootIter == this->root.begin()) {
                return { -1, { -1., -1 } };
            } else {
                --rootIter;
            }
            
            return {
                rootIter->second->orderId,
                {
                    rootIter->second->price,
                    rootIter->second->volume
                }
            };
        }
    };



    struct PQBuyCmp {
        // sort by volume, i.e. pair::second
        //using is_transparent = void;

        bool operator()(
            const std::pair<unsigned, unsigned> & a,
            const std::pair<unsigned, unsigned> & b
        ) const {
            if(a.second > b.second) {
                return true;
            } else if(a.second == b.second) {
                if(a.first < b.first) {
                    return true;
                }
            }

            return false;
        }
    };

    std::map<
        std::string,
        std::set<
            std::pair<unsigned, unsigned>,
            PQBuyCmp
        >
    > buyMap;

    std::map<unsigned, unsigned> lastBuyVolume;



    std::map<std::string, std::vector<Order>> orders;

    std::map<std::string, unsigned> activeOrdersPerSymbol;

    std::map<std::string, PriceTournamentStruct> sellMap;
    std::map<std::string, unsigned> ordinalsPerSymbol;
    std::map<std::string, std::map<unsigned, unsigned>> sellMapOrdinals;

public:
    Storage() = default;

private:
    void bookOrder(Order && order);

public:
    void retrieveOrder(const std::string & csvStrOrder);
    const auto & ordersCounts() const;
    const auto biggestBuyOrders(const std::string & symbol);
    const auto bestSellAtTime(const std::string & symbol, const std::string & timestamp);

};



void Storage::retrieveOrder(const std::string & csvStrOrder) {
    char timestampCstr[Order::STR_MAX_SIZE];
    char symbolCstr[Order::STR_MAX_SIZE];
    unsigned orderId;
    char operation;
    char sideCstr[Order::STR_MAX_SIZE];
    unsigned volume;
    double price;

    sscanf(csvStrOrder.c_str(), "%[^;]%*c%[^;]%*c%u%*c%c%*c%[^;]%*c%u%*c%lf",
        timestampCstr, symbolCstr, &orderId, &operation, sideCstr, &volume, &price
    );

    this->bookOrder(
        Order(
            orderId,
            operation,
            symbolCstr,
            sideCstr,
            volume,
            price,
            timestampCstr
        )
    );
}



void Storage::bookOrder(Order && order) {
    if(order.getOperation() == 'I') {
        ++this->activeOrdersPerSymbol[order.getSymbol()];
    } else if(order.getOperation() == 'C') {
        --this->activeOrdersPerSymbol[order.getSymbol()];
    }

    if(order.getSide() == "SELL") {
        this->orders[order.getSymbol()].push_back(order);

        auto iter = this->sellMap.find(order.getSymbol());

        if(iter == this->sellMap.end()) {
            iter = this->sellMap.insert({ order.getSymbol(), PriceTournamentStruct() }).first;
        }

        if(order.getOperation() == 'I') {
            iter->second.insert(order.getId(), this->ordinalsPerSymbol[order.getSymbol()], order.getPrice(), order.getVolume(), order.getTimestamp());
            this->sellMapOrdinals[order.getSymbol()][order.getId()] = this->ordinalsPerSymbol[order.getSymbol()];
            ++this->ordinalsPerSymbol[order.getSymbol()];
        } else if(order.getOperation() == 'A') {
            iter->second.amend(order.getId(), this->sellMapOrdinals[order.getSymbol()][order.getId()], order.getPrice(), order.getVolume(), order.getTimestamp());
        } else {
            iter->second.cancel(this->sellMapOrdinals[order.getSymbol()][order.getId()], order.getTimestamp());
        }
    } else {
        if(order.getOperation() == 'I') {
            this->buyMap[order.getSymbol()].insert({ order.getId(), order.getVolume() });
            this->lastBuyVolume[order.getId()] = order.getVolume();
        } else if(order.getOperation() == 'A') {
            this->buyMap[order.getSymbol()].erase({ order.getId(), this->lastBuyVolume[order.getId()] });
            this->buyMap[order.getSymbol()].insert({ order.getId(), order.getVolume() });
            this->lastBuyVolume[order.getId()] = order.getVolume();
        } else {
            this->buyMap[order.getSymbol()].erase({ order.getId(), this->lastBuyVolume[order.getId()] });
        }
    }
}



const auto & Storage::ordersCounts() const {
    return this->activeOrdersPerSymbol;
}


const auto Storage::biggestBuyOrders(const std::string & symbol) {
    std::vector<std::pair<unsigned, unsigned>> retVec;

    auto iter = this->buyMap[symbol].begin();

    for(int i = 0; i < 3  &&  iter != this->buyMap[symbol].end(); ++i, ++iter) {
        retVec.push_back(*iter);
    }

    return retVec;
}


const auto Storage::bestSellAtTime(const std::string & symbol, const std::string & timestamp) {
    return this->sellMap[symbol].getBest(timestamp);
}



#endif // STORAGE_H