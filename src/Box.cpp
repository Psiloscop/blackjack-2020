#include "Box.h"

Box::Box(Player* player, u8 allowedMaxValue)
    : player{player}, allowedMaxValue{allowedMaxValue}
{}

void Box::assignPlayer(Player* _player)
{
    this->player = _player;
}

Player& Box::getPlayer() const
{
    return *this->player;
}

void Box::resetBox()
{
    this->hands.clear();
    this->bets.clear();

    this->bets = {0};
    this->activeHand = 0;
}

void Box::giveCard(Card* card)
{
    if (this->hands.empty() || this->activeHand >= this->hands.size())
    {
        this->hands.resize(this->hands.size() + 1);
        this->hands[this->activeHand] = {};
    }

    this->hands[this->activeHand].push_back(card);
}

std::vector<std::vector<Card*>>& Box::getAllCards()
{
    return this->hands;
}

std::vector<Card*>& Box::getHandCards()
{
    return this->hands[this->activeHand];
}

u8 Box::getHandCount() const
{
    return this->hands.size();
}

u8 Box::getCurrentHandNumber() const
{
    return this->activeHand + 1;
}


u8 Box::getHandCardsCount()
{
    if (this->hands.empty())
    {
        return 0;
    }

    return this->hands[this->activeHand].size();
}


u8 Box::getHandCardsValue()
{
    if (this->hasBlackjack())
    {
        return 21;
    }

    u16 value = 0;
    u8 aceCardCount = 0;

    if (this->hands.empty())
    {
        return 0;
    }

    for (auto& card : this->hands[this->activeHand])
    {
        if (card->getCardFace() != CardFace::ace)
        {
            value += card->getCardValue();
        }
        else
        {
            aceCardCount++;
        }
    }

    if (aceCardCount == 1 && (this->allowedMaxValue - value) == Card::getMaxAceCardValue())
    {
        value += Card::getMaxAceCardValue();
    }
    else
    {
        while (aceCardCount > 0)
        {
            value += (this->allowedMaxValue - (value + Card::getMinAceCardValue() * aceCardCount) >= Card::getMaxAceCardValue())
                     ? Card::getMaxAceCardValue() : Card::getMinAceCardValue();

            aceCardCount--;
        }
    }

    return value;
}

bool Box::isAllowedMaxValueReached()
{
    return this->getHandCardsValue() >= this->allowedMaxValue;
}

void Box::switchHand(u8 number)
{
    if (number > this->hands.size() + 1 || number <= 0)
    {
        throw std::out_of_range("Box::switchHand(number) - number is out of range");
    }

    this->activeHand = number - 1;
}

void Box::setBet(u32 value, bool add)
{
    if (value > this->player->getCash())
    {
        throw std::overflow_error("Box::setBet(value, add) - value is bigger than user's cash: " +
            std::to_string(this->player->getCash()));
    }

    if (this->activeHand == this->bets.size())
    {
        this->bets.push_back(value);
    }
    else
    {
        this->bets[this->activeHand] = add ? this->bets[this->activeHand] + value : value;
    }

    this->player->decreaseCash(value);
}

void Box::updateBet(u32 value)
{
    if (this->activeHand == this->bets.size())
    {
        this->bets.push_back(value);
    }
    else
    {
        this->bets[this->activeHand] = value;
    }
}

u32 Box::getBet()
{
    return this->bets[this->activeHand];
}

u32 Box::getAllBets()
{
    u32 betSum = 0;
    u8 betCount = this->bets.size();

    for (u8 index = 0; index < betCount; index++)
    {
        betSum += this->bets[index];
    }

    return betSum;
}

std::vector<u8> Box::getPlayableHandNumbers()
{
    std::vector<u8> playableHands;

    u8 handCount = this->hands.size();
    u8 _activeHand = this->activeHand;

    for (u8 index = 0; index < handCount; index++)
    {
        this->activeHand = index;

        if (this->getHandCardsValue() <= this->allowedMaxValue)
        {
            playableHands.push_back(index + 1);
        }
    }

    this->activeHand = _activeHand;

    return playableHands;
}

bool Box::isBoxInSplit()
{
    return this->hands.size() > 1;
}

bool Box::hasBlackjack()
{
    if (this->hands.empty() || this->hands.size() > 1)
    {
        return false;
    }

    auto& cards = this->hands[this->activeHand];

    return cards.size() == 2 && (cards[0]->getCardValue() == 10 && cards[1]->getCardValue() == 11 ||
        cards[1]->getCardValue() == 10 && cards[0]->getCardValue() == 11);
}

bool Box::hasOvertake(bool checkAllHands)
{
    if (checkAllHands)
    {
        u8 handCount = this->hands.size();
        u8 _activeHand = this->activeHand;
        u8 overtakeCount = 0;
        bool overtakeOnActiveHand = false;

        for (u8 index = 0; index < handCount; index++)
        {
            this->activeHand = index;

            if (this->getHandCardsValue() > this->allowedMaxValue)
            {
                overtakeCount++;

                if (this->activeHand == _activeHand)
                {
                    overtakeOnActiveHand = true;
                }
            }
        }

        if (overtakeOnActiveHand)
        {
            if (overtakeCount == handCount)
            {
                this->activeHand = _activeHand;

                return true;
            }
            else
            {
                auto playableHandNumbers = this->getPlayableHandNumbers();

                if (playableHandNumbers.size() == 1 || playableHandNumbers[playableHandNumbers.size() - 1] == _activeHand)
                {
                    this->switchHand(playableHandNumbers[0]);
                }
                else
                {
                    auto it = std::find_if(playableHandNumbers.begin(), playableHandNumbers.end(),
                            [_activeHand](const u8 number) { return number > _activeHand; });

                    this->switchHand(*it);
                }

                return false;
            }
        }
        else
        {
            this->activeHand = _activeHand;

            return false;
        }
    }
    else
    {
        return this->getHandCardsValue() > this->allowedMaxValue;
    }
}