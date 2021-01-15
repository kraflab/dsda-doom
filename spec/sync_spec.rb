RSpec.describe 'sync' do
  let(:pwad) { nil }

  before do
    Utility.play_demo(lmp: lmp, pwad: pwad)
  end

  describe 'total time' do
    subject { Utility.read_levelstat.total }

    # complevel 2
    context 'doom2 30uv in 17:55 by Looper' do
      let(:lmp) { '30uv1755.lmp' }

      it { is_expected.to eq('17:55') }
    end

    # coop
    context 'doom2 20 uv max in 2:22 by termrork & kOeGy (a)' do
      let(:lmp) { 'cm20k222.lmp' }

      it { is_expected.to eq('2:22') }
    end

    context 'doom2 20 uv max in 2:22 by termrork & kOeGy (b)' do
      let(:lmp) { 'cm20t222.lmp' }

      it { is_expected.to eq('2:22') }
    end

    # complevel 9
    context 'rush 12 uv max in 21:14 by Ancalagon' do
      let(:lmp) { 'ru12-2114.lmp' }
      let(:pwad) { 'rush.wad' }

      it { is_expected.to eq('21:14') }
    end

    # complevel 11
    context 'valiant e1 uv speed in 5:13 by Krankdud' do
      let(:lmp) { 'vae1-513.lmp' }
      let(:pwad) { 'Valiant.wad' }

      it { is_expected.to eq('5:13') }
    end
  end
end
