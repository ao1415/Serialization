#include <Siv3D.hpp>

class Object {
private:

protected:


public:

	const size_t ObjectId;

	Object(const size_t objectId) : ObjectId(objectId) { }
	virtual ~Object() {};

	virtual bool update() = 0;
	virtual void draw() const = 0;

	virtual void SIV3D_SERIALIZE(Serializer<BinaryWriter>& archive) = 0;
	virtual void SIV3D_SERIALIZE(Deserializer<BinaryReader>& archive) = 0;

};

template<class Class>
size_t createTypeHash() { return typeid(Class).hash_code(); }

class Factory {
private:

	std::unordered_map<size_t, std::function<std::unique_ptr<Object>()>> createFunction;

public:

	template<class Class>
	void regist() {

		const auto hash = createTypeHash<Class>();

		assert(!createFunction.contains(hash));

		createFunction.insert({ hash, [hash]() {
			return std::make_unique<Class>(hash);
			} });

	}

	template<class Class>
	[[nodiscard]] std::unique_ptr<Object> create() const {

		const auto hash = createTypeHash<Class>();

		assert(createFunction.contains(hash));

		return createFunction.at(hash)();
	}

	[[nodiscard]] std::unique_ptr<Object> create(const size_t objectId) const {

		assert(createFunction.contains(objectId));

		return createFunction.at(objectId)();
	}

};

namespace State {

	void save(std::list<std::unique_ptr<Object>>& objects, const FilePath& path) {

		Serializer<BinaryWriter> writer(path);

		writer(objects.size());

		for (auto& o : objects)
		{
			writer(o->ObjectId);
			writer((*o));
		}

	}

	void load(std::list<std::unique_ptr<Object>>& objects, const FilePath& path, const Factory& factory) {

		objects.clear();

		Deserializer<BinaryReader> reader(path);

		size_t size;
		reader(size);

		for (size_t i = 0; i < size; i++)
		{
			size_t objectId;
			reader(objectId);

			auto obj = factory.create(objectId);

			reader((*obj));

			objects.push_back(std::move(obj));
		}

	}

};

class EffectRed : public Object {
private:

	Vec2 pos;
	Vec2 acceleration;
	int life = 300;

public:

	EffectRed(const size_t objectId) : Object(objectId) {
		pos = Scene::CenterF();
		acceleration = RandomVec2(1);
	}

	bool update() override {

		pos += acceleration;
		life--;

		return (life > 0);
	}

	void draw() const override {

		Circle(pos, 16).draw(Palette::Red);
		//FontAsset(U"Debug")(ToHex(ObjectId)).drawAt(pos);

	}

	void SIV3D_SERIALIZE(Serializer<BinaryWriter>& archive) override {
		archive(pos, acceleration, life);
	}
	void SIV3D_SERIALIZE(Deserializer<BinaryReader>& archive) override {
		archive(pos, acceleration, life);
	}

};

class EffectBlue : public Object {
private:

	Vec2 pos;
	Vec2 acceleration;
	int life = 300;

public:

	EffectBlue(const size_t objectId) : Object(objectId) {
		pos = Scene::CenterF();
		acceleration = RandomVec2(1);
	}

	bool update() override {

		pos += acceleration;
		life--;

		return (life > 0);
	}

	void draw() const override {

		Circle(pos, 16).draw(Palette::Blue);
		//FontAsset(U"Debug")(ToHex(ObjectId)).drawAt(pos);

	}

	void SIV3D_SERIALIZE(Serializer<BinaryWriter>& archive) override {
		archive(pos, acceleration, life);
	}
	void SIV3D_SERIALIZE(Deserializer<BinaryReader>& archive) override {
		archive(pos, acceleration, life);
	}

};

void Main() {

	std::list<std::unique_ptr<Object>> object;

	Factory factory;
	factory.regist<EffectRed>();
	factory.regist<EffectBlue>();

	FontAsset::Register(U"Debug", 12);
	FontAsset::Register(U"Debug2", 8);

	while (System::Update())
	{
		switch (Random(6))
		{
		case 0:
			object.push_back(factory.create<EffectRed>());
			break;
		case 1:
			object.push_back(factory.create<EffectBlue>());
			break;
		default:
			break;
		}

		for (auto it = object.begin(); it != object.end();)
		{
			if ((*it)->update())
			{
				it++;
			}
			else
			{
				it = object.erase(it);
			}
		}

		for (const auto& o : object) o->draw();


		ClearPrint();
		Print << object.size();
		Print << sizeof(SFMT19937_64) << U", " << sizeof(sfmt::SFMT_T);
		sfmt::SFMT_T randomState;

		std::memcpy(&randomState, &GetDefaultRNG(), sizeof(sfmt::SFMT_T));
		Print << randomState.idx;
		for (int i = 0; i < 156; i += 4)
		{
			String line = U"";
			line += ToHex(randomState.state[i + 0].u64[0]) + ToHex(randomState.state[i + 0].u64[1]);
			line += ToHex(randomState.state[i + 1].u64[0]) + ToHex(randomState.state[i + 1].u64[1]);
			line += ToHex(randomState.state[i + 2].u64[0]) + ToHex(randomState.state[i + 2].u64[1]);
			line += ToHex(randomState.state[i + 3].u64[0]) + ToHex(randomState.state[i + 3].u64[1]);
			FontAsset(U"Debug2")(line).draw(0, 8 * i / 4);
		}

		if ((KeyShift + Key1).down())
		{
			State::save(object, U"save1.dat");
		}
		else if (Key1.down())
		{
			State::load(object, U"save1.dat", factory);
		}

	}

}

//
// = アドバイス =
// Debug ビルドではプログラムの最適化がオフになります。
// 実行速度が遅いと感じた場合は Release ビルドを試しましょう。
// アプリをリリースするときにも、Release ビルドにするのを忘れないように！
//
// 思ったように動作しない場合は「デバッグの開始」でプログラムを実行すると、
// 出力ウィンドウに詳細なログが表示されるので、エラーの原因を見つけやすくなります。
//
// = お役立ちリンク =
//
// OpenSiv3D リファレンス
// https://siv3d.github.io/ja-jp/
//
// チュートリアル
// https://siv3d.github.io/ja-jp/tutorial/basic/
//
// よくある間違い
// https://siv3d.github.io/ja-jp/articles/mistakes/
//
// サポートについて
// https://siv3d.github.io/ja-jp/support/support/
//
// Siv3D Slack (ユーザコミュニティ) への参加
// https://siv3d.github.io/ja-jp/community/community/
//
// 新機能の提案やバグの報告
// https://github.com/Siv3D/OpenSiv3D/issues
//
