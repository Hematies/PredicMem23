#pragma once
#include "Global.h"

template<typename InType, typename OutType>
class Encoder {
public:
    virtual vector<OutType> encode(vector<InType>& input) = 0;
    virtual vector<vector<OutType>> encode(vector<vector<InType>>& input) = 0;
    virtual void fit(vector<vector<InType>>& input, vector<OutType>& output) = 0;
    virtual void copyTo(std::shared_ptr<Encoder<InType, OutType>>& encoder) = 0;
    virtual void clean() = 0;
};

template <typename encoder_t>
vector<float> adaptSequenceForSVM(encoder_t& encoder, vector<uint8_t>& sequence) {
    return encoder.encode(sequence);
}


template<typename InType, typename OutType>
class OneHotEncoder : public Encoder<InType, OutType> {
protected:
    uint16_t numClasses;
public:
    OneHotEncoder() {
        this->numClasses = 1;
    }

    OneHotEncoder(InType numClasses) {
        this->numClasses = numClasses;
    }

    OneHotEncoder(const OneHotEncoder<InType, OutType>& encoder) {
        this->numClasses = numClasses;
    }

    void copyTo(std::shared_ptr<Encoder<InType, OutType>>& encoder) {
        encoder = std::shared_ptr<Encoder<InType, OutType>>(
            (Encoder<InType, OutType>*) new OneHotEncoder<InType, OutType>(*this)
        );
    }

    void clean() {};

    vector<OutType> encode(vector<InType>& input) {
        auto input_ = vector<vector<InType>>{ input };
        return this->encode(input_)[0];
    }

    vector<vector<OutType>> encode(vector<vector<InType>>& input) {
        auto sampleSize = input[0].size();
        auto output = vector<vector<OutType>>(input.size(),
            vector<OutType>(sampleSize * numClasses, 0));
        for (int i = 0; i < input.size(); i++) {
            for (int j = 0; j < sampleSize; j++) {
                output[i][j * numClasses + input[i][j]] = (OutType)1;
            }
        }
        return output;
    }

    void fit(vector<vector<InType>>& input, vector<OutType>& output) {
        InType maxClass = 0;

        for (auto& inputSample : input) {
            for (InType& class_ : inputSample) {
                if (class_ > maxClass)
                    maxClass = class_;
            }
        }

        this->numClasses = maxClass + 1;
    }
};