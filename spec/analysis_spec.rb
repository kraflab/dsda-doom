RSpec.describe 'analysis' do
  let(:analysis)  { Utility.read_analysis }
  let(:pwad) { 'analysis_test.wad' }

  before do
    Utility.play_demo(lmp: lmp, pwad: pwad)
  end

  describe 'skill' do
    subject { analysis.skill }

    context 'doom2 map 4 nm speed by Vile' do
      let(:pwad) { nil }
      let(:lmp) { 'nm04-036.lmp' }

      it { is_expected.to eq(5) }
    end

    context 'doom2 map 1 uv speed by Thomas Pilger' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-005.lmp' }

      it { is_expected.to eq(4) }
    end
  end

  describe 'nomonsters' do
    subject { analysis.nomonsters? }

    context 'doom2 map 1 nomonsters by depr4vity' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01o497.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 1 uv speed by Thomas Pilger' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-005.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe 'respawn' do
    subject { analysis.respawn? }

    context 'doom2 map 2 uv respawn by Looper' do
      let(:pwad) { nil }
      let(:lmp) { 're02-107.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 4 nm speed by Vile' do
      let(:pwad) { nil }
      let(:lmp) { 'nm04-036.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe 'fast' do
    subject { analysis.fast? }

    context 'doom2 map 4 uv fast by Radek Pecka' do
      let(:pwad) { nil }
      let(:lmp) { 'fa04-109.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 4 nm speed by Vile' do
      let(:pwad) { nil }
      let(:lmp) { 'nm04-036.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe '100k' do
    subject { analysis.hundred_k? }

    context 'doom2 map 1 uv max by Xit Vono' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-039.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 1 uv speed by Thomas Pilger' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-005.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe '100s' do
    subject { analysis.hundred_s? }

    context 'doom2 episode 1 nm100s in 11:56 by JCD' do
      let(:pwad) { nil }
      let(:lmp) { '1156ns01.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 1 uv speed by Thomas Pilger' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-005.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe 'missed things' do
    let(:missed_monsters) { analysis.missed_monsters }
    let(:missed_secrets) { analysis.missed_secrets }

    context 'doom2 ep 3 max in 26:54 by Vile' do
      let(:pwad) { nil }
      let(:lmp) { 'lve3-2654.lmp' }

      it 'misses one secret (map 27)' do
        expect(missed_monsters).to eq(0)
        expect(missed_secrets).to eq(1)
      end
    end
  end

  describe 'pacifist' do
    subject { analysis.pacifist? }

    context 'when there is a barrel chain' do
      let(:lmp) { 'barrel_chain.lmp' }

      it { is_expected.to eq(false) }
    end

    context 'when there is a barrel assist' do
      let(:lmp) { 'barrel_assist.lmp' }

      it { is_expected.to eq(false) }
    end

    context 'when the player shoots a keen' do
      let(:lmp) { 'keen.lmp' }

      it { is_expected.to eq(false) }
    end

    context 'when the player shoots a romero' do
      let(:lmp) { 'romero.lmp' }

      it { is_expected.to eq(false) }
    end

    context 'when there is splash damage' do
      let(:lmp) { 'splash.lmp' }

      it { is_expected.to eq(false) }
    end

    context 'when there is a telefrag' do
      let(:lmp) { 'telefrag.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'when the player shoots a voodoo doll' do
      let(:lmp) { 'voodoo.lmp' }

      it { is_expected.to eq(true) }
    end
  end

  describe 'stroller' do
    subject { analysis.stroller? }

    context 'doom2 map 8 stroller by 4shockblast' do
      let(:pwad) { nil }
      let(:lmp) { 'lv08str037.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 8 pacifist by 4shockblast' do
      let(:pwad) { nil }
      let(:lmp) { 'pa08-020.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe 'reality' do
    let(:reality) { analysis.reality? }
    let(:almost_reality) { analysis.almost_reality? }

    context 'when the player takes enemy damage' do
      let(:lmp) { 'damage.lmp' }

      it 'is not reality' do
        expect(reality).to eq(false)
        expect(almost_reality).to eq(false)
      end
    end

    context 'when the player takes nukage damage' do
      let(:lmp) { 'nukage.lmp' }

      it 'is almost reality' do
        expect(reality).to eq(false)
        expect(almost_reality).to eq(true)
      end
    end

    context 'when the player takes crusher damage' do
      let(:lmp) { 'crusher.lmp' }

      it 'is not reality' do
        expect(reality).to eq(false)
        expect(almost_reality).to eq(false)
      end
    end

    context 'when the player takes no damage' do
      let(:lmp) { 'reality.lmp' }

      it 'is reality' do
        expect(reality).to eq(true)
        expect(almost_reality).to eq(false)
      end
    end
  end

  describe 'tyson weapons' do
    subject { analysis.tyson_weapons? }

    context 'doom2 map 1 tyson by j4rio' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01t040.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 1 uv max by Xit Vono' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-039.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe 'turbo' do
    subject { analysis.turbo? }

    context 'doom2 done turbo quicker by 4shockblast' do
      let(:pwad) { nil }
      let(:lmp) { 'd2dtqr.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 1 uv max by Xit Vono' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01-039.lmp' }

      it { is_expected.to eq(false) }
    end
  end

  describe 'weapon_collector' do
    subject { analysis.weapon_collector? }

    context 'doom2 map 1 collector by hokis' do
      let(:pwad) { nil }
      let(:lmp) { 'cl01-022.lmp' }

      it { is_expected.to eq(true) }
    end

    context 'doom2 map 1 tyson by j4rio' do
      let(:pwad) { nil }
      let(:lmp) { 'lv01t040.lmp' }

      it { is_expected.to eq(false) }
    end
  end
end
