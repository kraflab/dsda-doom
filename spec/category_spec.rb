RSpec.describe 'category' do
  let(:analysis)  { Utility.read_analysis }
  let(:pwad) { nil }

  before do
    Utility.play_demo(lmp: lmp, pwad: pwad)
  end

  subject { analysis.category }

  context 'doom2 map 1 uv speed by Thomas Pilger' do
    let(:pwad) { nil }
    let(:lmp) { 'lv01-005.lmp' }

    it { is_expected.to eq('UV Speed') }
  end

  context 'doom2 map 1 uv max by Xit Vono' do
    let(:pwad) { nil }
    let(:lmp) { 'lv01-039.lmp' }

    it { is_expected.to eq('UV Max') }
  end

  context 'doom2 map 1 tyson by j4rio' do
    let(:pwad) { nil }
    let(:lmp) { 'lv01t040.lmp' }

    it { is_expected.to eq('UV Tyson') }
  end

  context 'doom2 map 8 pacifist by 4shockblast' do
    let(:pwad) { nil }
    let(:lmp) { 'pa08-020.lmp' }

    it { is_expected.to eq('Pacifist') }
  end

  context 'doom2 map 8 stroller by 4shockblast' do
    let(:pwad) { nil }
    let(:lmp) { 'lv08str037.lmp' }

    it { is_expected.to eq('Stroller') }
  end

  context 'doom2 map 4 nm speed by Vile' do
    let(:pwad) { nil }
    let(:lmp) { 'nm04-036.lmp' }

    it { is_expected.to eq('NM Speed') }
  end

  context 'doom2 episode 1 nm100s in 11:56 by JCD' do
    let(:pwad) { nil }
    let(:lmp) { '1156ns01.lmp' }

    it { is_expected.to eq('NM 100S') }
  end

  context 'doom2 map 1 nomonsters by depr4vity' do
    let(:pwad) { nil }
    let(:lmp) { 'lv01o497.lmp' }

    it { is_expected.to eq('NoMo') }
  end

  context 'doom2 map 1 nomo100s by 4shockblast' do
    let(:pwad) { nil }
    let(:lmp) { 'os01-2394.lmp' }

    it { is_expected.to eq('NoMo 100S') }
  end

  context 'doom2 map 2 uv respawn by Looper' do
    let(:pwad) { nil }
    let(:lmp) { 're02-107.lmp' }

    it { is_expected.to eq('UV Respawn') }
  end

  context 'doom2 map 4 uv fast by Radek Pecka' do
    let(:pwad) { nil }
    let(:lmp) { 'fa04-109.lmp' }

    it { is_expected.to eq('UV Fast') }
  end

  context 'doom2 done turbo quicker by 4shockblast' do
    let(:pwad) { nil }
    let(:lmp) { 'd2dtqr.lmp' }

    it { is_expected.to eq('Other') }
  end
end
