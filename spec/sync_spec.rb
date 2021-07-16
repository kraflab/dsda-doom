RSpec.describe 'sync' do
  let(:pwad) { nil }
  let(:iwad) { "DOOM2.WAD" }
  let(:extra) { nil }

  before do
    Utility.play_demo(lmp: lmp, iwad: iwad, pwad: pwad, extra: extra)
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

    # heretic
    context 'heretic' do
      let(:iwad) { "DOOM.WAD" }
      let(:pwad) { "HERETIC.WAD" }
      let(:extra) { "-heretic" }

      context 'e1 sm max in 52:40 by JCD' do
        let(:lmp) { 'h1m-5240.lmp' }

        it { is_expected.to eq('52:40') }
      end

      context 'e2 sm max in 67:02 by JCD' do
        let(:lmp) { 'h2ma6702.lmp' }

        it { is_expected.to eq('67:02') }
      end

      context 'e3 sm max in 62:48 by JCD' do
        let(:lmp) { 'h3ma6248.lmp' }

        it { is_expected.to eq('62:48') }
      end

      context 'e4 sm speed in 10:55 by veovis' do
        let(:lmp) { 'h4sp1055.lmp' }

        it { is_expected.to eq('10:55') }
      end

      context 'e5 sm speed in 12:57 by veovis' do
        let(:lmp) { 'h5sp1257.lmp' }

        it { is_expected.to eq('12:57') }
      end
    end

    # hexen
    context 'hexen' do
      let(:iwad) { "HEXEN.WAD" }
      let(:extra) { "-hexen" }

      context "e1 sk4 max in 45:37 by PVS" do
        let(:lmp) { "me1c4537.lmp" }

        it { is_expected.to eq("45:37") }
      end
    end
  end
end
